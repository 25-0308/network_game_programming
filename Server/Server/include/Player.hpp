#ifndef __player_hpp
#define __player_hpp

#include "common.hpp"
#include "Vec2.hpp"
#include "AABB.hpp"

// 서버가 들고 있는 플레이어 상태
// 클라이언트 Player에서 Animator, Image, ResourceHandler, Collider(GDI), InputHandler 의존을 뺀 것
// Object를 상속하지 않고 값으로 들고 다님 (게임 로직 스레드만 접근하므로 락 없음)
// 이동, 점프, 대시는 update에서 처리 (11/14)
// 발차기, 피격, 기절, 밀어내기, 고정 dt 틱은 (11/15)추가할 예정
class Player {
public:
	// 경기 시작 시 한 번 호출: 플레이어 번호와 캐릭터로 스탯을 정하고 HP를 채움
	void init( std::uint8_t playerNum, Protocol::Character character );

	// 골 이후 킥오프 위치로 (클라 Player::reset과 같이 HP, 기절은 그대로 둠)
	void resetToKickoff( );

	// 한 틱 진행 (클라 Player::update의 이동, 점프, 대시 부분)
	// 이 틱까지 받은 입력을 쓰고, 끝나면 눌림, 뗌 기록을 비움
	void update( float dt );

	// ---------------- 입력 ----------------
	// 클라 InputHandler의 Tap / Hold / Away를 비트로 대신함
	// 틱 사이에 눌렀다 뗀 키도 놓치지 않도록 눌림, 뗌을 다음 틱까지 모아 둠

	// C2S_Input을 받을 때마다 호출
	void applyInput( std::uint8_t keys );

	// 누르고 있음 (Tap 또는 Hold)
	bool isHeld( Protocol::InputKey key ) const {
		return ( keys_ & key ) != 0;
	}
	// 지난 틱 이후 새로 누름 (Tap)
	bool isTapped( Protocol::InputKey key ) const {
		return ( tapped_ & key ) != 0;
	}
	// 지난 틱 이후 뗌 (Away)
	bool isReleased( Protocol::InputKey key ) const {
		return ( released_ & key ) != 0;
	}

	// 한 틱 처리가 끝나면 호출 (update가 마지막에 부름)
	void clearInputEdges( ) {
		tapped_ = 0;
		released_ = 0;
	}

	// ---------------- 충돌 영역 ----------------

	// 몸통 박스, 위치에서 바로 계산하므로 Collider::update로 맞춰 줄 필요 없음
	AABB body( ) const {
		return { pos_ + Vec2( 0.f, GameConst::playerBodyOffsetY ),
			Vec2( GameConst::playerBodyWidth, GameConst::playerBodyHeight ) };
	}

	// 1P는 오른쪽(+1), 2P는 왼쪽(-1)을 보고 섬
	float facing( ) const {
		return playerNum_ == 1 ? 1.f : -1.f;
	}

	// 발차기 판정 기준점
	Vec2 foot( ) const {
		return pos_ + Vec2( GameConst::kickFootOffsetX * facing( ), GameConst::kickFootOffsetY );
	}

	// ---------------- 애니메이션 ----------------

	// 이동 애니메이션 지정, 발차기 모션 중에는 바꾸지 않음 (클라 Player::playAnim)
	void setMoveAnim( Protocol::PlayerAnim anim ) {
		if ( kickAnimT_ <= 0.f ) {
			anim_ = anim;
		}
	}

	// ---------------- 상태 조회 ----------------

	std::uint8_t playerNum( ) const {
		return playerNum_;
	}
	Protocol::Character character( ) const {
		return character_;
	}
	Vec2 pos( ) const {
		return pos_;
	}
	float hp( ) const {
		return hp_;
	}
	bool isStunned( ) const {
		return stunT_ > 0.f;
	}
	float speed( ) const {
		return speed_;
	}
	float jumpPower( ) const {
		return jumpPower_;
	}
	float kickDamage( ) const {
		return kickDamage_;
	}

	// S2C_GameState에 넣을 스냅샷
	Protocol::PlayerState toState( ) const;

private:
	// update 단계
	void applyGravity( float dt );
	void tickTimers( float dt );
	void tryJump( );

	// 한 방향 키의 대시 판정, 대시 이동, 걷기를 처리
	// dir: 왼쪽 -1, 오른쪽 +1
	void moveHorizontal( Protocol::InputKey key, float dir,
		float& myTapTimer, float& otherTapTimer, bool& dashing, float dt );

	void clampToField( );

	// 고정 정보
	std::uint8_t playerNum_ = 0;	// 1(왼쪽) 또는 2(오른쪽)
	Protocol::Character character_ = Protocol::Character::America;
	float speed_ = 0.f;			// 클라 speed
	float jumpPower_ = 0.f;		// 클라 jumppower (위쪽이 -)
	float kickDamage_ = 0.f;	// 클라 kickstat * damagePerKickStat

	// 위치, 점프
	Vec2 pos_;
	float jspeed_ = 0.f;		// 점프로 생긴 세로 속도
	bool jumped_ = false;		// 공중에 있음

	// 대시
	float lTapTimer_ = 0.f;		// 클라 ldtimer_, 왼쪽 두 번째 뗌을 기다리는 시간
	float rTapTimer_ = 0.f;		// 클라 rdtimer_
	float dashTimer_ = 0.f;		// 클라 dashtimer, 대시 남은 시간
	float dashCooldown_ = 0.f;	// 클라 dashintime, 다음 대시까지
	bool lDashing_ = false;		// 클라 ldashed
	bool rDashing_ = false;		// 클라 rdashed

	// 체력, 발차기
	float hp_ = GameConst::playerMaxHp;
	float stunT_ = 0.f;			// 남은 기절 시간
	float kickCooldown_ = 0.f;
	float kickAnimT_ = 0.f;		// 남은 발차기 모션 시간, 0보다 크면 anim은 Kick
	Protocol::PlayerAnim anim_ = Protocol::PlayerAnim::Idle;

	// 입력
	std::uint8_t keys_ = 0;		// 지금 누르고 있는 키
	std::uint8_t tapped_ = 0;	// 지난 틱 이후 새로 누른 키
	std::uint8_t released_ = 0;	// 지난 틱 이후 뗀 키
};

#endif	// __player_hpp
