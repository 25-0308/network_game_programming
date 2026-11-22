#ifndef __client_network_hpp
#define __client_network_hpp

#include "common.hpp"

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <queue>

#include "Protocol.hpp"

// 서버 접속과 패킷 수신 (논블로킹 소켓, 게임 루프에서 poll)
class ClientNetwork {
public:
	// 윈속 초기화 후 서버에 접속, 실패 시 false
	static bool connect( const wchar_t* ip, std::uint16_t port );
	static void release( );

	// 도착한 데이터를 모두 읽고 완성된 패킷을 큐에 쌓음, 연결이 끊기면 false
	static bool poll( );

	// 받은 패킷 하나를 꺼냄, 없으면 false
	static bool popPacket( std::vector<char>& packet );

	// 패킷을 보냄, 송신 버퍼가 차서 못 보낸 나머지는 다음 sendPacket이나 poll에서 이어 보냄
	// 연결이 끊기면 false
	static bool sendPacket( const void* packet, int size );

	static bool isConnected( ) {
		return sock_ != INVALID_SOCKET;
	}

	static std::uint8_t getPlayerNum( ) {
		return playerNum_;
	}
	static void setPlayerNum( std::uint8_t playerNum ) {
		playerNum_ = playerNum;
	}

private:
	// sendBuf_에 남은 데이터를 보낼 수 있는 만큼 보냄, 연결이 끊기면 false
	static bool flushSend( );

	static SOCKET sock_;
	static std::vector<char> recvBuf_;	// 아직 패킷이 안 된 데이터
	static std::vector<char> sendBuf_;	// 아직 못 보낸 데이터
	static std::queue<std::vector<char>> packets_;
	static std::uint8_t playerNum_;
};

#endif	// __client_network_hpp
