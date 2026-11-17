#ifndef __network_handler_hpp
#define __network_handler_hpp

#include "common.hpp"

// common.hpp에서 WIN32_LEAN_AND_MEAN으로 Windows.h가 winsock.h를 안 끌어오므로 뒤에 포함해도 됨
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

#include "Protocol.hpp"

// 받을 수 있는 패킷 최대 크기 (서버 common.hpp의 maxPacketSize와 같음)
constexpr int maxPacketSize = 64;

// 수신 스레드가 받은 패킷 하나
struct RecvPacket {
	std::array<char, maxPacketSize> data;
	int size;
};

// 윈속 초기화, 서버 접속, 수신 스레드, 송신
// 수신 스레드는 패킷을 큐에 쌓기만 하고, 처리는 메인 스레드의 update에서 함
class NetworkHandler {
public:
	static bool init( );
	static void release( );

	// socket, connect 후 수신 스레드 시작, 실패 시 false
	static bool connect( const wchar_t* ip, std::uint16_t port = Protocol::serverPort );

	// 수신 큐에 쌓인 패킷을 메인 스레드에서 처리
	static void update( );

	// 패킷 전체를 보냄, 실패 시 false
	static bool sendPacket( const void* packet, int size );

	static bool isConnected( ) {
		return connected_;
	}

	// 서버가 배정한 플레이어 번호, 1(왼쪽) 또는 2(오른쪽), 받기 전에는 0
	static std::uint8_t getPlayerNum( ) {
		return playerNum_;
	}

private:
	static void recvThread( );

	// 패킷 하나(헤더 + 본문)를 buffer에 받음
	// 성공 시 받은 바이트 수, 실패나 접속 종료 시 -1
	static int recvPacket( char* buffer, int bufferSize );

	// len 바이트를 다 받을 때까지 recv
	static bool recvAll( char* buffer, int len );

	static void processPacket( const RecvPacket& packet );

	static void printError( const char* msg );

	static bool wsaInit_;
	static SOCKET sock_;
	static std::thread recvThread_;

	static std::atomic<bool> connected_;
	static std::atomic<bool> running_;	// release에서 끈 뒤 수신 실패는 오류로 출력하지 않음

	static std::mutex queueMutex_;
	static std::queue<RecvPacket> recvQueue_;

	static std::uint8_t playerNum_;
};

#endif	// __network_handler_hpp
