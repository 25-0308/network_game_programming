#ifndef __network_hpp
#define __network_hpp

#include "common.hpp"

#include <atomic>
#include <functional>

// 윈속 초기화, 리슨 소켓, 송수신 도우미 (소켓은 모두 블로킹)
class Network {
public:
	static bool init( );
	static void release( );

	// socket, bind, listen
	static bool listen( std::uint16_t port );

	// [종료 처리] 리슨 소켓을 닫아 메인 스레드의 막힌 accept를 깨움
	static void closeListen( );

	// [메인] 접속이 올 때까지 기다림 (블로킹), 실패나 리슨 소켓을 닫으면 INVALID_SOCKET
	static SOCKET accept( );

	// 패킷 전체를 보냄, 실패 시 false
	static bool sendPacket( SOCKET sock, const void* packet, int size );

	// [플레이어] 패킷 하나(헤더 + 본문)를 buffer에 받음
	// 성공 시 받은 바이트 수, 실패, 접속 종료, shouldStop이 true면 -1
	// 다른 스레드의 shutdown, CancelIoEx로는 막힌 recv가 깨어나지 않으므로
	// select로 recvPollMs씩만 기다리며 그 사이마다 shouldStop을 확인함
	static int recvPacket( SOCKET sock, char* buffer, int bufferSize, const std::function<bool( )>& shouldStop );

	static void printError( const char* msg );

private:
	// len 바이트를 다 받을 때까지 recv
	static bool recvAll( SOCKET sock, char* buffer, int len, const std::function<bool( )>& shouldStop );

	// 메인(accept)과 종료 처리(closeListen)가 같이 쓰므로 atomic
	static std::atomic<SOCKET> listenSock_;
};

#endif	// __network_hpp
