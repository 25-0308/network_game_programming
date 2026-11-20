#ifndef __game_logic_hpp
#define __game_logic_hpp

#include "common.hpp"
#include "JobQueue.hpp"
#include "Match.hpp"

// 게임 전체 흐름: 매 틱 전체 작업큐를 순서대로 실행 -> 경기 진행 -> 클라에 전송
// 경기: 2명 접속 시 시작 -> 시간이 끝나면 종료 (도중에 나가면 중단)
// 소켓 닫기와 패킷 처리는 여기서만 함 (지금은 메인 스레드, 추후 게임 로직 스레드)
class GameLogic {
public:
	// 타이머 시작, 첫 틱 예약
	static void init( );

	// 전체 작업큐에 추가 (접속, 수신 쪽에서 호출)
	static void pushJob( const Job& job );

	// 다음 틱까지 남은 시간 (초), 0 이하면 틱을 돌릴 차례
	static double timeUntilNextTick( );

	// 한 틱 진행
	static void tick( );

private:
	// 쌓인 작업을 넣은 순서대로 모두 실행
	static void processJobs( );
	static void processJob( const Job& job );

	static void onPlayerJoin( std::uint8_t playerNum );
	static void onPlayerLeave( std::uint8_t playerNum );
	static void onPacket( std::uint8_t playerNum, const char* data, int size );

	// 두 캐릭터로 경기 시작
	static void startMatch( );

	// 시간 갱신, 경기 중이면 한 틱 진행
	static void update( );

	// 이번 틱 결과를 클라에 전송
	static void render( );

	static JobQueue jobQueue_;
	static double nextTick_;

	static Match match_;
	static bool playing_;	// 경기 진행 중
	static bool ended_;		// 이번 틱에 시간이 끝남 (render에서 S2C_GameEnd 전송)

	// 플레이어별 확정한 캐릭터 (C2S_SelectCharacter를 받기 전에는 America)
	static std::array<Protocol::Character, Protocol::maxPlayers> characters_;
	static std::array<std::uint8_t, Protocol::maxPlayers> scores_;
};

#endif	// __game_logic_hpp
