#ifndef __game_constants_hpp
#define __game_constants_hpp

#include <array>
#include <cstdint>

#include "Protocol.hpp"

// 서버와 클라이언트가 같이 쓰는 경기 수치
// 값은 현재 클라이언트 코드(Player, Ball, GoalManager, InGameScene, ImageStorage)에서 그대로 가져옴
// 좌표는 모두 화면(1280x720) 기준 픽셀, 위치는 오브젝트 중심, 시간은 초, 속도는 px/s

namespace GameConst {

	// ---------------- 화면, 경기장 ----------------

	constexpr float screenWidth = 1280.f;
	constexpr float screenHeight = 720.f;

	// 플레이어와 공이 벽에 막히는 좌우 경계
	constexpr float fieldLeft = 0.f;
	constexpr float fieldRight = 1280.f;

	// ---------------- 경기 진행 ----------------

	constexpr std::uint16_t matchTime = 120;	// 경기 시간 (2분)
	constexpr float goalScreenTime = 3.f;		// 골 화면 후 킥오프까지
	constexpr std::uint8_t stageCount = 4;		// 경기장 1 ~ 4

	// 킥오프 위치 (InGameScene::entry, GoalManager::kickoff)
	constexpr float ballStartX = 640.f;
	constexpr float ballStartY = 360.f;
	constexpr float p1StartX = 300.f;
	constexpr float p1StartY = 500.f;
	constexpr float p2StartX = 980.f;
	constexpr float p2StartY = 500.f;

	// ---------------- 골 판정 ----------------

	// 공 중심이 골라인을 넘고 크로스바 아래(y >= crossbarBottom)면 골
	// 왼쪽 골대에 들어가면 2P 득점, 오른쪽이면 1P 득점
	constexpr float leftGoalLine = 50.f;
	constexpr float rightGoalLine = 1230.f;
	constexpr float crossbarBottom = 420.f;

	// 크로스바 충돌 박스 (골대 위치 + Collider offset을 계산한 중심 좌표)
	constexpr float crossbarWidth = 70.f;
	constexpr float crossbarHeight = 30.f;
	constexpr float leftCrossbarX = 35.f;		// -20 + 55
	constexpr float rightCrossbarX = 1245.f;	// 1300 - 55
	constexpr float crossbarY = 390.f;			// 550 - 160

	// ---------------- 플레이어 ----------------

	constexpr float playerMaxHp = 100.f;

	// 클라이언트 Player는 scale_이 (0, 0)이라 중심 좌표가 그대로 지면, 벽에 걸림
	constexpr float playerGroundY = 580.f;		// 서 있을 때 중심 y

	// 몸통 충돌 박스 (AABB), 중심은 위치 + offset
	constexpr float playerBodyWidth = 70.f;
	constexpr float playerBodyHeight = 90.f;
	constexpr float playerBodyOffsetY = 15.f;

	// 낙하: 매 프레임 일정 속도로 내려가고, 점프 속도(jspeed)에는 따로 중력이 붙음
	constexpr float playerFallSpeed = 1000.f;	// 항상 더해지는 하강 속도
	constexpr float playerGravity = 1000.f;		// jspeed 가속도 (px/s^2)

	// 대시: 같은 방향 키를 0.3초 안에 두 번 떼면 0.075초 동안 100px 이동
	constexpr float dashTapWindow = 0.3f;
	constexpr float dashTime = 0.075f;
	constexpr float dashDistance = 100.f;
	constexpr float dashSpeed = dashDistance / dashTime;
	constexpr float dashCooldown = 0.5f;

	// 발차기
	constexpr float kickCooldown = 0.5f;
	constexpr std::uint8_t kickFrameCnt = 8;
	constexpr float kickFrameTime = 0.03f;
	constexpr float kickAnimTime = kickFrameCnt * kickFrameTime;	// 0.24초
	constexpr float kickFootOffsetX = 30.f;	// 1P는 +, 2P는 - (서로 마주 봄)
	constexpr float kickFootOffsetY = 50.f;
	constexpr float kickBallRange = 90.f;	// 발 ~ 공 중심 거리
	constexpr float kickHitReach = 40.f;	// 발 ~ 상대 몸통 박스 거리
	constexpr float stunTime = 2.f;			// HP 0이 되면 기절, 끝나면 HP 회복

	// ---------------- 캐릭터 스탯 ----------------

	struct CharacterStat {
		int speed;
		int kick;
		int jump;
	};

	// Protocol::Character 순서 (ImageStorage::statData와 같음)
	constexpr std::array<CharacterStat, 4> characterStats{ {
		{ 8, 5, 5 },	// America
		{ 5, 8, 5 },	// Argentina
		{ 7, 5, 7 },	// France
		{ 6, 6, 6 },	// Korea
	} };

	constexpr const CharacterStat& statOf( Protocol::Character character ) {
		return characterStats[ static_cast<std::size_t>( character ) ];
	}

	// 스탯 -> 실제 수치 (Player 생성자, Player::kick)
	constexpr float speedFromStat( int speedStat ) {
		return speedStat * 50.f;
	}
	constexpr float jumpPowerFromStat( int jumpStat ) {
		return -( 1250.f + 40.f * jumpStat );	// 위쪽이 -y
	}
	constexpr float kickDamageFromStat( int kickStat ) {
		return kickStat * 3.5f;	// 5 -> 17.5 (6대), 6 -> 21 (5대), 8 -> 28 (4대)
	}

	// ---------------- 공 ----------------

	constexpr float ballRadius = 35.f;		// ball.png 70x70
	constexpr float ballGravity = 1000.f;
	constexpr float ballFrictionPerSec = 0.7f;	// 1초에 가로 속도 70% 유지, vx *= pow( 0.7, dt )

	constexpr float ballStartVx = 1.f;
	constexpr float ballStartVy = -100.f;

	// 벽, 천장, 바닥 (공 중심 기준으로 radius를 더하고 빼서 비교)
	constexpr float ballGroundY = 650.5f;	// 공 아래쪽 끝이 닿는 바닥선
	constexpr float ballCeilingY = 10.f;	// 공 위쪽 끝이 이 선에 닿으면 튕김
	constexpr float ballCeilingResetY = 20.f;	// 튕길 때 위쪽 끝을 여기로 옮김
	constexpr float ballGroundRestitution = 0.8f;	// 바닥에서 vy 감쇠

	// 플레이어, 크로스바와 충돌 (원 vs 박스)
	constexpr float ballBoxRestitution = 0.8f;
	constexpr float ballPlayerMinPush = 300.f;	// 멈춘 공도 움직이는 플레이어에게 밀리도록
	constexpr float ballCrossbarMinPush = 0.f;	// 크로스바 위에서는 굴러가거나 멈출 수 있음

	// 발차기로 더해지는 속도 (kickright는 +x, kickleft는 -x)
	constexpr float ballKickVx = 2000.f;
	constexpr float ballKickVy = -1000.f;

}	// namespace GameConst

#endif	// __game_constants_hpp
