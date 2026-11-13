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
	if ( listenSock_ != INVALID_SOCKET ) {
		closesocket( listenSock_ );
		listenSock_ = INVALID_SOCKET;
	}
	WSACleanup( );
}

bool Network::listen( std::uint16_t port ) {
	listenSock_ = socket( AF_INET, SOCK_STREAM, 0 );
	if ( listenSock_ == INVALID_SOCKET ) {
		printError( "socket" );
		return false;
	}

	auto addr = sockaddr_in{ };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( INADDR_ANY );
	addr.sin_port = htons( port );

	if ( bind( listenSock_, reinterpret_cast<sockaddr*>( &addr ), sizeof( addr ) ) == SOCKET_ERROR ) {
		printError( "bind" );
		return false;
	}

	if ( ::listen( listenSock_, SOMAXCONN ) == SOCKET_ERROR ) {
		printError( "listen" );
		return false;
	}

	return true;
}

bool Network::waitAccept( double seconds ) {
	fd_set readSet;
	FD_ZERO( &readSet );
	FD_SET( listenSock_, &readSet );

	auto micro = static_cast<long>( seconds * 1'000'000.0 );
	auto timeout = timeval{ micro / 1'000'000, micro % 1'000'000 };

	// 리슨 소켓이 읽기 가능하면 접속 요청이 있음
	int ret = select( 0, &readSet, nullptr, nullptr, &timeout );
	if ( ret == SOCKET_ERROR ) {
		printError( "select" );
		return false;
	}
	return ret > 0;
}

SOCKET Network::accept( ) {
	auto addr = sockaddr_in{ };
	int addrLen = sizeof( addr );

	auto sock = ::accept( listenSock_, reinterpret_cast<sockaddr*>( &addr ), &addrLen );
	if ( sock == INVALID_SOCKET ) {
		printError( "accept" );
		return INVALID_SOCKET;
	}

	// 작은 패킷을 자주 보내므로 Nagle 끄기
	BOOL noDelay = TRUE;
	setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>( &noDelay ), sizeof( noDelay ) );

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

int Network::recvPacket( SOCKET sock, char* buffer, int bufferSize ) {
	constexpr int headerSize = sizeof( Protocol::PacketHeader );

	// 헤더 먼저 받고 size만큼 나머지를 받음
	if ( !recvAll( sock, buffer, headerSize ) ) {
		return -1;
	}

	auto header = reinterpret_cast<Protocol::PacketHeader*>( buffer );
	if ( header->size < headerSize || header->size > bufferSize ) {
		std::cout << "잘못된 패킷 크기: " << header->size << "\n";
		return -1;
	}

	if ( !recvAll( sock, buffer + headerSize, header->size - headerSize ) ) {
		return -1;
	}

	return header->size;
}

void Network::printError( const char* msg ) {
	std::cout << "[오류] " << msg << " (" << WSAGetLastError( ) << ")\n";
}

bool Network::recvAll( SOCKET sock, char* buffer, int len ) {
	int received = 0;

	while ( received < len ) {
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

SOCKET Network::listenSock_ = INVALID_SOCKET;
