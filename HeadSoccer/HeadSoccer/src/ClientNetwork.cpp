#include "../include/ClientNetwork.hpp"

#include <cstdio>

#pragma comment( lib, "ws2_32" )

namespace {
	// 받을 수 있는 패킷 최대 크기
	constexpr std::uint16_t maxPacketSize = 64;
}

bool ClientNetwork::connect( const wchar_t* ip, std::uint16_t port ) {
	WSADATA wsa;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 ) {
		return false;
	}

	sock_ = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sock_ == INVALID_SOCKET ) {
		release( );
		return false;
	}

	auto addr = sockaddr_in{ };
	addr.sin_family = AF_INET;
	addr.sin_port = htons( port );
	if ( InetPtonW( AF_INET, ip, &addr.sin_addr ) != 1 ) {
		release( );
		return false;
	}

	if ( ::connect( sock_, reinterpret_cast<sockaddr*>( &addr ), sizeof( addr ) ) == SOCKET_ERROR ) {
		release( );
		return false;
	}

	// 게임 루프가 멈추지 않도록 논블로킹으로
	u_long nonBlocking = 1;
	ioctlsocket( sock_, FIONBIO, &nonBlocking );

	return true;
}

void ClientNetwork::release( ) {
	if ( sock_ != INVALID_SOCKET ) {
		closesocket( sock_ );
		sock_ = INVALID_SOCKET;
	}
	WSACleanup( );
}

bool ClientNetwork::poll( ) {
	char buffer[ 512 ];

	// 지금 도착한 데이터를 전부 읽음
	while ( true ) {
		int ret = recv( sock_, buffer, sizeof( buffer ), 0 );
		if ( ret > 0 ) {
			recvBuf_.insert( recvBuf_.end( ), buffer, buffer + ret );
			continue;
		}
		if ( ret == 0 ) {
			return false;	// 서버가 연결을 닫음
		}
		if ( WSAGetLastError( ) == WSAEWOULDBLOCK ) {
			break;			// 더 읽을 데이터 없음
		}
		return false;
	}

	// 헤더의 size만큼 모였으면 패킷 하나로 떼어냄
	constexpr std::size_t headerSize = sizeof( Protocol::PacketHeader );
	while ( recvBuf_.size( ) >= headerSize ) {
		auto header = reinterpret_cast<const Protocol::PacketHeader*>( recvBuf_.data( ) );
		if ( header->size < headerSize || header->size > maxPacketSize ) {
			std::printf( "잘못된 패킷 크기: %u\n", header->size );
			return false;
		}
		if ( recvBuf_.size( ) < header->size ) {
			break;
		}

		packets_.emplace( recvBuf_.begin( ), recvBuf_.begin( ) + header->size );
		recvBuf_.erase( recvBuf_.begin( ), recvBuf_.begin( ) + header->size );
	}

	return true;
}

bool ClientNetwork::popPacket( std::vector<char>& packet ) {
	if ( packets_.empty( ) ) {
		return false;
	}
	packet = std::move( packets_.front( ) );
	packets_.pop( );
	return true;
}

SOCKET ClientNetwork::sock_ = INVALID_SOCKET;
std::vector<char> ClientNetwork::recvBuf_;
std::queue<std::vector<char>> ClientNetwork::packets_;
std::uint8_t ClientNetwork::playerNum_ = 0;
