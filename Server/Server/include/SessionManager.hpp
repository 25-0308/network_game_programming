#ifndef __session_manager_hpp
#define __session_manager_hpp

#include "common.hpp"

// 접속한 클라이언트 한 명
struct Session {
	SOCKET sock = INVALID_SOCKET;
	bool connected = false;
};

// 두 플레이어 자리 관리
class SessionManager {
public:
	// 빈 자리에 등록하고 플레이어 번호(1, 2) 반환, 꽉 찼으면 0
	static std::uint8_t add( SOCKET sock );

	// 소켓을 닫고 자리를 비움
	static void remove( std::uint8_t playerNum );

	static SOCKET getSocket( std::uint8_t playerNum );
	static bool isConnected( std::uint8_t playerNum );
	static int count( );

	// 연결된 모든 플레이어에게 보냄
	static void broadcast( const void* packet, int size );

private:
	static std::array<Session, Protocol::maxPlayers> sessions_;
};

#endif	// __session_manager_hpp
