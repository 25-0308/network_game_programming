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

	// 최대 seconds 동안 새 접속이나 플레이어 데이터를 기다림 (수신 스레드 전환 전까지 사용)
	// listenReady: 접속 요청 있음, ready[i]: socks[i]에 받을 데이터 있음 (INVALID_SOCKET은 건너뜀)
	static void waitReadable( double seconds, const std::array<SOCKET, Protocol::maxPlayers>& socks,
		bool& listenReady, std::array<bool, Protocol::maxPlayers>& ready );

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
