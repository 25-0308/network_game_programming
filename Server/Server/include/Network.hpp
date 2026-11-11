#ifndef __network_hpp
#define __network_hpp

#include "common.hpp"

// 윈속 초기화, 리슨 소켓, 송수신 도우미
class Network {
public:
	static bool init( );
	static void release( );

	// socket, bind, listen
	static bool listen( std::uint16_t port );

	// 접속 대기, 실패 시 INVALID_SOCKET
	static SOCKET accept( );

	// 패킷 전체를 보냄, 실패 시 false
	static bool sendPacket( SOCKET sock, const void* packet, int size );

	// 패킷 하나(헤더 + 본문)를 buffer에 받음
	// 성공 시 받은 바이트 수, 실패나 접속 종료 시 -1
	static int recvPacket( SOCKET sock, char* buffer, int bufferSize );

	static void printError( const char* msg );

private:
	// len 바이트를 다 받을 때까지 recv
	static bool recvAll( SOCKET sock, char* buffer, int len );

	static SOCKET listenSock_;
};

#endif	// __network_hpp
