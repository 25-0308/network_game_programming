#include "../include/Network.hpp"

bool Network::init( ) {
	WSADATA wsa;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 ) {
		std::cout << "WSAStartup 실패\n";
		return false;
	}
	return true;
}

void Network::release( ) {
	closeListen( );
	WSACleanup( );
}

bool Network::listen( std::uint16_t port ) {
	auto sock = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sock == INVALID_SOCKET ) {
		printError( "socket" );
		return false;
	}
	listenSock_ = sock;

	auto addr = sockaddr_in{ };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( INADDR_ANY );
	addr.sin_port = htons( port );

	if ( bind( sock, reinterpret_cast<sockaddr*>( &addr ), sizeof( addr ) ) == SOCKET_ERROR ) {
		printError( "bind" );
		return false;
	}

	if ( ::listen( sock, SOMAXCONN ) == SOCKET_ERROR ) {
		printError( "listen" );
		return false;
	}

	return true;
}

void Network::closeListen( ) {
	// 두 번 닫지 않도록 꺼내면서 비움
	auto sock = listenSock_.exchange( INVALID_SOCKET );
	if ( sock != INVALID_SOCKET ) {
		closesocket( sock );
	}
}

SOCKET Network::accept( ) {
	auto addr = sockaddr_in{ };
	int addrLen = sizeof( addr );

	auto sock = ::accept( listenSock_, reinterpret_cast<sockaddr*>( &addr ), &addrLen );
	if ( sock == INVALID_SOCKET ) {
		// 종료하려고 리슨 소켓을 닫은 경우는 오류가 아님
		if ( listenSock_ != INVALID_SOCKET ) {
			printError( "accept" );
		}
		return INVALID_SOCKET;
	}

	// 작은 패킷을 자주 보내므로 Nagle 끄기
	BOOL noDelay = TRUE;
	setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>( &noDelay ), sizeof( noDelay ) );

	// 읽지 않는 클라 때문에 send가 계속 막혀 게임 로직 스레드가 멈추지 않도록
	DWORD sendTimeout = sendTimeoutMs;
	setsockopt( sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>( &sendTimeout ), sizeof( sendTimeout ) );

	char ip[ INET_ADDRSTRLEN ]{ };
	inet_ntop( AF_INET, &addr.sin_addr, ip, sizeof( ip ) );
	std::cout << "[접속] " << ip << ":" << ntohs( addr.sin_port ) << "\n";

	return sock;
}

bool Network::sendPacket( SOCKET sock, const void* packet, int size ) {
	auto data = static_cast<const char*>( packet );
	int sent = 0;

	while ( sent < size ) {
		int ret = send( sock, data + sent, size - sent, 0 );
		if ( ret == SOCKET_ERROR ) {
			printError( "send" );
			return false;
		}
		sent += ret;
	}
	return true;
}

int Network::recvPacket( SOCKET sock, char* buffer, int bufferSize, const std::function<bool( )>& shouldStop ) {
	constexpr int headerSize = sizeof( Protocol::PacketHeader );

	// 헤더 먼저 받고 size만큼 나머지를 받음
	if ( !recvAll( sock, buffer, headerSize, shouldStop ) ) {
		return -1;
	}

	auto header = reinterpret_cast<Protocol::PacketHeader*>( buffer );
	if ( header->size < headerSize || header->size > bufferSize ) {
		std::cout << "잘못된 패킷 크기: " << header->size << "\n";
		return -1;
	}

	if ( !recvAll( sock, buffer + headerSize, header->size - headerSize, shouldStop ) ) {
		return -1;
	}

	return header->size;
}

void Network::printError( const char* msg ) {
	std::cout << "[오류] " << msg << " (" << WSAGetLastError( ) << ")\n";
}

bool Network::recvAll( SOCKET sock, char* buffer, int len, const std::function<bool( )>& shouldStop ) {
	int received = 0;

	while ( received < len ) {
		// 받을 데이터가 올 때까지 recvPollMs씩 기다리며 멈추라는 요청을 확인
		fd_set readSet;
		FD_ZERO( &readSet );
		FD_SET( sock, &readSet );
		auto timeout = timeval{ 0, recvPollMs * 1000 };

		int ready = select( 0, &readSet, nullptr, nullptr, &timeout );
		if ( ready == SOCKET_ERROR ) {
			printError( "select" );
			return false;
		}
		if ( ready == 0 ) {
			if ( shouldStop( ) ) {
				return false;
			}
			continue;
		}

		// 읽을 수 있으므로 막히지 않음 (연결 종료면 0)
		int ret = recv( sock, buffer + received, len - received, 0 );
		if ( ret == SOCKET_ERROR ) {
			printError( "recv" );
			return false;
		}
		// 상대가 연결을 정상 종료
		if ( ret == 0 ) {
			return false;
		}
		received += ret;
	}
	return true;
}

std::atomic<SOCKET> Network::listenSock_ = INVALID_SOCKET;
