#include "../include/NetworkHandler.hpp"

#include <iostream>

bool NetworkHandler::init( ) {
	WSADATA wsa;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 ) {
		std::cout << "WSAStartup 실패\n";
		return false;
	}
	wsaInit_ = true;
	return true;
}

void NetworkHandler::release( ) {
	running_ = false;

	// 윈도우에서는 shutdown으로 다른 스레드의 막힌 recv가 깨어나지 않으므로 소켓을 먼저 닫고 기다림
	// sock_ 값은 스레드가 끝난 뒤에 지움 (수신 스레드가 읽는 중)
	if ( sock_ != INVALID_SOCKET ) {
		closesocket( sock_ );
	}
	if ( recvThread_.joinable( ) ) {
		recvThread_.join( );
	}
	sock_ = INVALID_SOCKET;
	connected_ = false;

	if ( wsaInit_ ) {
		WSACleanup( );
		wsaInit_ = false;
	}
}

bool NetworkHandler::connect( const wchar_t* ip, std::uint16_t port ) {
	if ( sock_ != INVALID_SOCKET ) {
		return connected_;
	}

	auto addr = sockaddr_in{ };
	addr.sin_family = AF_INET;
	addr.sin_port = htons( port );
	if ( InetPtonW( AF_INET, ip, &addr.sin_addr ) != 1 ) {
		char ipStr[64]{ };
		WideCharToMultiByte( CP_ACP, 0, ip, -1, ipStr, sizeof( ipStr ) - 1, nullptr, nullptr );
		std::cout << "잘못된 서버 주소: " << ipStr << "\n";
		return false;
	}

	sock_ = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sock_ == INVALID_SOCKET ) {
		printError( "socket" );
		return false;
	}

	if ( ::connect( sock_, reinterpret_cast<sockaddr*>( &addr ), sizeof( addr ) ) == SOCKET_ERROR ) {
		printError( "connect" );
		closesocket( sock_ );
		sock_ = INVALID_SOCKET;
		return false;
	}

	// 입력 패킷처럼 작은 패킷을 자주 보내므로 Nagle 끄기
	BOOL noDelay = TRUE;
	setsockopt( sock_, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>( &noDelay ), sizeof( noDelay ) );

	std::cout << "서버 접속 성공 (포트 " << port << ")\n";

	connected_ = true;
	running_ = true;
	recvThread_ = std::thread( recvThread );

	return true;
}

void NetworkHandler::update( ) {
	// 락은 큐를 옮겨 담는 동안만 잡고, 처리는 락 밖에서 함
	auto packets = std::queue<RecvPacket>{ };
	{
		std::lock_guard<std::mutex> lock( queueMutex_ );
		std::swap( packets, recvQueue_ );
	}

	while ( !packets.empty( ) ) {
		processPacket( packets.front( ) );
		packets.pop( );
	}
}

bool NetworkHandler::sendPacket( const void* packet, int size ) {
	if ( !connected_ ) {
		return false;
	}

	auto data = static_cast<const char*>( packet );
	int sent = 0;

	while ( sent < size ) {
		int ret = send( sock_, data + sent, size - sent, 0 );
		if ( ret == SOCKET_ERROR ) {
			printError( "send" );
			return false;
		}
		sent += ret;
	}
	return true;
}

void NetworkHandler::recvThread( ) {
	auto packet = RecvPacket{ };

	while ( running_ ) {
		packet.size = recvPacket( packet.data.data( ), maxPacketSize );
		if ( packet.size < 0 ) {
			break;
		}

		std::lock_guard<std::mutex> lock( queueMutex_ );
		recvQueue_.push( packet );
	}

	if ( running_ ) {
		std::cout << "서버와 연결이 끊어짐\n";
	}
	connected_ = false;
}

int NetworkHandler::recvPacket( char* buffer, int bufferSize ) {
	constexpr int headerSize = sizeof( Protocol::PacketHeader );

	// 헤더 먼저 받고 size만큼 나머지를 받음
	if ( !recvAll( buffer, headerSize ) ) {
		return -1;
	}

	auto header = reinterpret_cast<Protocol::PacketHeader*>( buffer );
	if ( header->size < headerSize || header->size > bufferSize ) {
		std::cout << "잘못된 패킷 크기: " << header->size << "\n";
		return -1;
	}

	if ( !recvAll( buffer + headerSize, header->size - headerSize ) ) {
		return -1;
	}

	return header->size;
}

bool NetworkHandler::recvAll( char* buffer, int len ) {
	int received = 0;

	while ( received < len ) {
		int ret = recv( sock_, buffer + received, len - received, 0 );
		if ( ret == SOCKET_ERROR ) {
			if ( running_ ) {
				printError( "recv" );
			}
			return false;
		}
		// 서버가 연결을 정상 종료
		if ( ret == 0 ) {
			return false;
		}
		received += ret;
	}
	return true;
}

void NetworkHandler::processPacket( const RecvPacket& packet ) {
	auto header = reinterpret_cast<const Protocol::PacketHeader*>( packet.data.data( ) );

	switch ( header->type ) {
	case Protocol::PacketType::S2C_Login: {
		auto login = reinterpret_cast<const Protocol::S2C_Login*>( packet.data.data( ) );
		if ( login->playerNum < 1 || login->playerNum > Protocol::maxPlayers ) {
			std::cout << "잘못된 플레이어 번호: " << static_cast<int>( login->playerNum ) << "\n";
			break;
		}
		playerNum_ = login->playerNum;
		std::cout << "플레이어 " << static_cast<int>( playerNum_ ) << " 배정\n";
		break;
	}

	default:
		break;
	}
}

void NetworkHandler::printError( const char* msg ) {
	std::cout << "[오류] " << msg << " (" << WSAGetLastError( ) << ")\n";
}

bool NetworkHandler::wsaInit_ = false;
SOCKET NetworkHandler::sock_ = INVALID_SOCKET;
std::thread NetworkHandler::recvThread_;

std::atomic<bool> NetworkHandler::connected_ = false;
std::atomic<bool> NetworkHandler::running_ = false;

std::mutex NetworkHandler::queueMutex_;
std::queue<RecvPacket> NetworkHandler::recvQueue_;

std::uint8_t NetworkHandler::playerNum_ = 0;
