#include "../include/ClientNetwork.hpp"

#include <cstdio>
#include <cstring>

#pragma comment( lib, "ws2_32" )

namespace {

	// 서버가 보내는 패킷의 정해진 크기, 모르는 타입은 0 (헤더의 size만 보고 건너뜀)
	constexpr std::uint16_t expectedSize( Protocol::PacketType type ) {
		using Protocol::PacketType;
		switch ( type ) {
		case PacketType::S2C_Login:				return sizeof( Protocol::S2C_Login );
		case PacketType::S2C_ChangeScene:		return sizeof( Protocol::S2C_ChangeScene );
		case PacketType::S2C_CharacterSelected:	return sizeof( Protocol::S2C_CharacterSelected );
		case PacketType::S2C_GameStart:			return sizeof( Protocol::S2C_GameStart );
		case PacketType::S2C_GameState:			return sizeof( Protocol::S2C_GameState );
		case PacketType::S2C_Ball:				return sizeof( Protocol::S2C_Ball );
		case PacketType::S2C_Goal:				return sizeof( Protocol::S2C_Goal );
		case PacketType::S2C_GameEnd:			return sizeof( Protocol::S2C_GameEnd );
		case PacketType::S2C_EnemyLeft:			return sizeof( Protocol::S2C_EnemyLeft );
		case PacketType::S2C_Frame:				return sizeof( Protocol::S2C_Frame );
		case PacketType::S2C_Collision:			return sizeof( Protocol::S2C_Collision );
		default:								return 0;
		}
	}
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
	sendBuf_.clear( );
	WSACleanup( );
}

bool ClientNetwork::poll( ) {
	// 지난번에 다 못 보낸 데이터부터 보냄
	if ( !flushSend( ) ) {
		return false;
	}

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

	// recv 한 번에 패킷 일부만 오거나 여러 개가 붙어 올 수 있으므로
	// 헤더의 size만큼 모였을 때만 패킷 하나로 떼어내고, 덜 온 나머지는 다음 poll까지 남겨 둠
	constexpr std::size_t headerSize = sizeof( Protocol::PacketHeader );
	std::size_t offset = 0;
	while ( recvBuf_.size( ) - offset >= headerSize ) {
		auto header = Protocol::PacketHeader{ };
		std::memcpy( &header, recvBuf_.data( ) + offset, headerSize );

		if ( header.size < headerSize || header.size > Protocol::maxPacketSize ) {
			std::printf( "잘못된 패킷 크기: %u (type %u)\n", header.size, static_cast<unsigned>( header.type ) );
			return false;
		}

		// 정해진 크기와 다르면 이후 바이트의 경계도 어긋나 있으므로 끊음
		auto expected = expectedSize( header.type );
		if ( expected != 0 && header.size != expected ) {
			std::printf( "패킷 크기 불일치: type %u, %u (기대 %u)\n",
				static_cast<unsigned>( header.type ), header.size, expected );
			return false;
		}

		if ( recvBuf_.size( ) - offset < header.size ) {
			break;
		}

		auto begin = recvBuf_.begin( ) + offset;
		packets_.emplace( begin, begin + header.size );
		offset += header.size;
	}

	// 쓴 앞부분은 한 번에 지움 (패킷마다 지우면 남은 데이터를 매번 앞으로 옮김)
	recvBuf_.erase( recvBuf_.begin( ), recvBuf_.begin( ) + offset );

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

bool ClientNetwork::sendPacket( const void* packet, int size ) {
	if ( sock_ == INVALID_SOCKET ) {
		return false;
	}

	// 앞서 못 보낸 데이터 뒤에 붙여야 순서가 유지됨
	auto data = static_cast<const char*>( packet );
	sendBuf_.insert( sendBuf_.end( ), data, data + size );
	return flushSend( );
}

bool ClientNetwork::flushSend( ) {
	std::size_t sent = 0;
	while ( sent < sendBuf_.size( ) ) {
		int ret = send( sock_, sendBuf_.data( ) + sent, static_cast<int>( sendBuf_.size( ) - sent ), 0 );
		if ( ret > 0 ) {
			sent += ret;
			continue;
		}
		if ( ret == SOCKET_ERROR && WSAGetLastError( ) == WSAEWOULDBLOCK ) {
			break;	// 송신 버퍼가 참, 나머지는 다음에 보냄
		}
		return false;
	}

	sendBuf_.erase( sendBuf_.begin( ), sendBuf_.begin( ) + sent );
	return true;
}

SOCKET ClientNetwork::sock_ = INVALID_SOCKET;
std::vector<char> ClientNetwork::recvBuf_;
std::vector<char> ClientNetwork::sendBuf_;
std::queue<std::vector<char>> ClientNetwork::packets_;
std::uint8_t ClientNetwork::playerNum_ = 0;
