#ifndef __match_hpp
#define __match_hpp

#include "common.hpp"
#include "Player.hpp"
#include "Ball.hpp"

// 경기 한 판의 상태와 고정 dt 틱 진행
// 클라이언트 InGameScene + CollisionHandler에서 player부분을 이전
// 게임 로직 스레드만 접근함 (입력은 그 스레드에서 applyInput으로 넣음)
class Match {
public:
	// 경기 시작, 두 캐릭터로 플레이어를 초기화
	void start( Protocol::Character p1Character, Protocol::Character p2Character );

	// C2S_Input을 받은 플레이어에게 키 상태 전달
	void applyInput( std::uint8_t playerNum, std::uint8_t keys );

	// GameConst::tickDt만큼 한 틱 진행
	void tick( );

	const Player& player( std::uint8_t playerNum ) const {
		return players_[ playerNum - 1 ];
	}

	const ServerGame::Ball& ball( ) const {
		return ball_;
	}

	// 이번 틱에 골이 났으면 득점자(1, 2)를 돌려주고 비움, 없으면 0
	std::uint8_t takeScorer( );

private:
	// kicker의 발차기를 상대에게 판정 (클라 Player::kick의 뒷부분)
	void resolveKick( Player& kicker, Player& target );

	// 골 화면 중이면 시간을 세고 끝나면 킥오프, 아니면 골 판정 (클라 GoalManager::update)
	void updateGoal( float dt );

	// 공과 두 플레이어를 킥오프 위치로 (클라 GoalManager::kickoff)
	void kickoff( );

	std::array<Player, Protocol::maxPlayers> players_;	// [0] = 1P, [1] = 2P
	ServerGame::Ball ball_;

	bool celebrating_ = false;	// 골 화면 중, 킥오프 전까지 새 골은 세지 않음
	float celebrateT_ = 0.f;	// 골 화면이 시작된 뒤 지난 시간
	std::uint8_t scorer_ = 0;	// 이번 틱 득점자 (takeScorer가 가져감)
};

#endif	// __match_hpp
