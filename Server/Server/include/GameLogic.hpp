#ifndef __game_logic_hpp
#define __game_logic_hpp

#include "common.hpp"
#include "JobQueue.hpp"
#include "Match.hpp"

// 게임 진행 단계
enum class Phase : std::uint8_t {
	Waiting,	// 상대 접속 대기 (1명)
	Select,		// 2명 접속, 캐릭터 선택과 준비
	Countdown,	// 둘 다 준비, startCountdown초 뒤 경기 시작
	Playing,	// 경기 중
};

// 게임 전체 흐름: 매 틱 전체 작업큐를 순서대로 실행 -> 단계 진행 -> 클라에 전송
// 대기 -> 선택 -> 카운트다운 -> 경기 -> (시간 종료) 선택, 도중에 나가면 대기로
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

	// 캐릭터 변경, 준비, 준비 취소
	static void onSelectCharacter( std::uint8_t playerNum, const Protocol::C2S_SelectCharacter& select );

	// 둘 다 준비했으면 카운트다운 시작
	static void tryStartCountdown( );

	// 한 플레이어의 캐릭터와 준비 상태를 두 명 모두에게 알림
	static void sendSelection( std::uint8_t playerNum );

	// 랜덤 스테이지와 두 캐릭터로 경기 시작
	static void startMatch( );

	// 시간 갱신, 단계별 진행 (카운트다운, 경기 한 틱)
	static void update( );

	// 이번 틱 결과를 클라에 전송
	static void render( );

	static JobQueue jobQueue_;
	static double nextTick_;

	static Phase phase_;
	static float countdownT_;	// 경기 시작까지 남은 시간

	static Match match_;
	static std::uint8_t stage_;	// 1 ~ stageCount
	static bool started_;		// 이번 틱에 경기 시작 (render에서 S2C_GameStart 전송)
	static bool ended_;			// 이번 틱에 시간이 끝남 (render에서 S2C_GameEnd 전송)

	// 플레이어별 선택 중인 캐릭터와 준비 상태
	static std::array<Protocol::Character, Protocol::maxPlayers> characters_;
	static std::array<bool, Protocol::maxPlayers> ready_;
	static std::array<std::uint8_t, Protocol::maxPlayers> scores_;
};

#endif	// __game_logic_hpp
