#ifndef __session_manager_hpp
#define __session_manager_hpp

#include "common.hpp"

enum class SlotState : std::uint8_t {
	Empty,		// 빈 자리
	Reserved,	// 메인이 배정하고 S2C_Login을 보내는 중 (아직 broadcast 대상 아님)
	Connected,	// 수신 스레드 실행 중
};

// 접속한 클라이언트 한 명
struct Session {
	SOCKET sock = INVALID_SOCKET;
	SlotState state = SlotState::Empty;
	bool sendFailed = false;	// 전송 실패, 이후로는 보내지 않음
	bool dropReported = false;	// 전송 실패를 게임 로직에 알렸는지
	bool stopRequested = false;	// 수신 스레드에 끝내라고 요청함
	HANDLE thread = nullptr;	// 이 자리의 수신 스레드
};

// 두 플레이어 자리 관리
// 메인(접속), 게임 로직(전송, 정리), 플레이어(소켓 읽기) 스레드가 같이 쓰므로 모든 함수가 락을 잡음
// 락 안에서는 send, 스레드 대기처럼 막힐 수 있는 일을 하지 않음
// 소켓을 닫는 곳은 remove, closeAll뿐이고 둘 다 수신 스레드가 끝난 뒤에 닫음
class SessionManager {
public:
	static void init( );

	// [메인] 빈 자리를 예약하고 플레이어 번호(1, 2) 반환, 꽉 찼으면 0
	static std::uint8_t reserve( SOCKET sock );

	// [메인] S2C_Login을 보낸 뒤 수신 스레드와 함께 등록, 이때부터 broadcast 대상
	static void activate( std::uint8_t playerNum, HANDLE thread );

	// [메인] 예약 취소 (S2C_Login 전송 실패 등), 소켓은 호출한 쪽이 닫음
	static void cancel( std::uint8_t playerNum );

	// [게임 로직] PlayerLeave 처리 때만 호출
	// 수신 스레드가 끝나길 기다린 뒤 소켓을 닫고 자리를 비움
	static void remove( std::uint8_t playerNum );

	// [메인] 서버 종료 시 게임 로직 스레드가 끝난 뒤 호출
	// 모든 수신 스레드를 멈추고 기다린 뒤 소켓을 닫음
	static void closeAll( );

	static SOCKET getSocket( std::uint8_t playerNum );
	static int count( );

	// [플레이어] 이 스레드가 끝나야 하는지 (recvPollMs마다 확인)
	static bool shouldStop( std::uint8_t playerNum );

	// [게임 로직] 접속 중인 모든 플레이어에게 보냄
	// 실패하면(끊김, 송신 타임아웃) 더 보내지 않고 그 수신 스레드에 멈추라고 요청함
	// 스레드가 끝나며 PlayerLeave를 넣으면 그때 자리를 정리함
	static void broadcast( const void* packet, int size );

	// [게임 로직] 전송에 실패한 뒤 아직 알리지 않은 플레이어 (한 번만 true)
	// 수신 스레드가 끝나기 전에 게임에서 먼저 퇴장 처리하는 데 씀
	static bool takeDropped( std::uint8_t playerNum );

private:
	static CRITICAL_SECTION lock_;
	static std::array<Session, Protocol::maxPlayers> sessions_;
};

#endif	// __session_manager_hpp
