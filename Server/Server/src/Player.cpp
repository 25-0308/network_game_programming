#include "../include/Player.hpp"

#include <algorithm>
#include <cmath>

void Player::init( std::uint8_t playerNum, Protocol::Character character ) {
	playerNum_ = playerNum;
	character_ = character;

	const auto& stat = GameConst::statOf( character );
	speed_ = GameConst::speedFromStat( stat.speed );
	jumpPower_ = GameConst::jumpPowerFromStat( stat.jump );
	kickDamage_ = GameConst::kickDamageFromStat( stat.kick );

	hp_ = GameConst::playerMaxHp;
	stunT_ = 0.f;
	keys_ = 0;
	clearInputEdges( );

	resetToKickoff( );
}

void Player::resetToKickoff( ) {
	pos_ = playerNum_ == 1
		? Vec2( GameConst::p1StartX, GameConst::p1StartY )
		: Vec2( GameConst::p2StartX, GameConst::p2StartY );

	jspeed_ = 0.f;
	jumped_ = false;
	lTapTimer_ = 0.f;
	rTapTimer_ = 0.f;
	dashTimer_ = 0.f;
	dashCooldown_ = 0.f;
	lDashing_ = false;
	rDashing_ = false;
	kickCooldown_ = 0.f;
	kickAnimT_ = 0.f;
	justKicked_ = false;
	anim_ = Protocol::PlayerAnim::Idle;
}

void Player::update( float dt ) {
	justKicked_ = false;

	// 클라 Player::update와 같은 순서: 중력 -> 타이머 -> (기절이 아니면) 발차기, 점프, 좌, 우 -> 벽
	applyGravity( dt );
	tickTimers( dt );

	if ( !isStunned( ) ) {
		tryKick( );
		tryJump( );
		moveHorizontal( Protocol::KeyLeft, -1.f, lTapTimer_, rTapTimer_, lDashing_, dt );
		moveHorizontal( Protocol::KeyRight, 1.f, rTapTimer_, lTapTimer_, rDashing_, dt );
	}

	clampToField( );

	// 기절 중에 누른 키도 버림 (클라도 기절 중 Tap은 사라짐)
	clearInputEdges( );
}

void Player::applyGravity( float dt ) {
	// 항상 일정 속도로 내려가고, 점프 속도에는 따로 중력이 붙음
	pos_.y += GameConst::playerFallSpeed * dt;
	pos_.y += jspeed_ * dt;
	jspeed_ += GameConst::playerGravity * dt;

	if ( pos_.y >= GameConst::playerGroundY ) {
		pos_.y = GameConst::playerGroundY;
		jumped_ = false;
		// 서 있는 동안 계속 커지면 다음 낙하가 최고 속도로 시작함
		jspeed_ = 0.f;
	}
}

void Player::tickTimers( float dt ) {
	lTapTimer_ -= dt;
	rTapTimer_ -= dt;
	dashCooldown_ -= dt;

	// 기절이 끝나면 HP를 다 채워서 복귀
	if ( isStunned( ) ) {
		stunT_ -= dt;
		if ( stunT_ <= 0.f ) {
			stunT_ = 0.f;
			hp_ = GameConst::playerMaxHp;
		}
	}

	kickCooldown_ -= dt;
	if ( kickAnimT_ > 0.f ) {
		kickAnimT_ -= dt;
		if ( kickAnimT_ <= 0.f ) {
			// 클라는 모션이 끝나면 idle을 재생하고, 키를 누르고 있으면 다음 이동에서 다시 달리기로 바뀜
			kickAnimT_ = 0.f;
			anim_ = Protocol::PlayerAnim::Idle;
		}
	}
}

void Player::tryKick( ) {
	// 클라 Player::kick의 앞부분 (쿨타임, 모션 시작)
	// 명중 판정은 상대와 공이 필요하므로 Match가 justKicked를 보고 처리
	if ( !isTapped( Protocol::KeyKick ) || kickCooldown_ > 0.f ) {
		return;
	}

	kickCooldown_ = GameConst::kickCooldown;
	kickAnimT_ = GameConst::kickAnimTime;
	justKicked_ = true;

	// update에서 중력, 타이머 다음, 점프와 좌우 이동 전에 불리므로 클라 kick()과 같은 시점의 발 위치
	kickFoot_ = foot( );
}

void Player::takeHit( float damage ) {
	if ( isStunned( ) || damage <= 0.f ) {
		return;
	}

	hp_ = std::max( hp_ - damage, 0.f );
	if ( hp_ > 0.f ) {
		return;
	}

	// 기절: stunT_가 끝날 때까지 움직이지 못하고, 끝나면 HP를 채워 복귀
	// 진행 중이던 대시와 발차기 모션은 끊음
	stunT_ = GameConst::stunTime;
	lDashing_ = false;
	rDashing_ = false;
	kickAnimT_ = 0.f;
	anim_ = Protocol::PlayerAnim::Idle;
}

void Player::separateFrom( Player& other ) {
	auto mine = body( );
	auto theirs = other.body( );

	auto overlap = mine.overlapAmount( theirs );
	if ( overlap.x <= 0.f || overlap.y <= 0.f ) {
		return;
	}

	auto delta = theirs.center - mine.center;

	if ( overlap.y < overlap.x ) {
		// 상대 머리 위에 착지: 그 위에 세우고 jumped_를 풀어 줌
		// 단, 점프를 누른 틱에도 중력으로 다시 겹쳐 여기서 jspeed_가 0이 되므로 머리 위에서는 점프가 취소됨 (클라와 같은 방식으로 처리)
		auto& upper = delta.y > 0.f ? *this : other;
		upper.pos_.y -= overlap.y;
		upper.jspeed_ = 0.f;
		upper.jumped_ = false;
	}
	else {
		// 옆으로 붙음: 겹친 길이의 반씩 서로 반대로 밀어냄
		float dir = delta.x >= 0.f ? 1.f : -1.f;
		pos_.x -= dir * overlap.x / 2.f;
		other.pos_.x += dir * overlap.x / 2.f;
	}
}

void Player::tryJump( ) {
	if ( !jumped_ && isTapped( Protocol::KeyJump ) ) {
		jspeed_ = jumpPower_;
		jumped_ = true;
	}
}

void Player::moveHorizontal( Protocol::InputKey key, float dir,
	float& myTapTimer, float& otherTapTimer, bool& dashing, float dt ) {
	// 대시: 같은 방향 키를 dashTapWindow 안에 두 번 떼면 시작
	if ( dashCooldown_ <= 0.f && isReleased( key ) ) {
		if ( myTapTimer <= 0.f ) {
			// 첫 번째 뗌, 반대쪽 대기는 취소
			myTapTimer = GameConst::dashTapWindow;
			otherTapTimer = 0.f;
		}
		else {
			dashing = true;
			myTapTimer = 0.f;
			dashTimer_ = GameConst::dashTime;
		}
	}

	if ( dashing ) {
		pos_.x += dir * GameConst::dashSpeed * dt;
		dashTimer_ -= dt;
		if ( dashTimer_ <= 0.f ) {
			dashCooldown_ = GameConst::dashCooldown;
			dashing = false;
		}
	}

	// 걷기: 누르고 있는 동안 이동, 뗀 틱에 Idle
	// 보고 있는 쪽으로 가면 RunFront, 반대쪽이면 RunBack
	if ( isHeld( key ) ) {
		pos_.x += dir * speed_ * dt;
		setMoveAnim( dir * facing( ) > 0.f ? Protocol::PlayerAnim::RunFront : Protocol::PlayerAnim::RunBack );
	}
	else if ( isReleased( key ) ) {
		setMoveAnim( Protocol::PlayerAnim::Idle );
	}
}

void Player::clampToField( ) {
	// 몸통 폭은 따지지 않고 중심 x가 경기장 경계에 걸림 (원래 게임과 같음)
	if ( pos_.x <= GameConst::fieldLeft ) {
		pos_.x = GameConst::fieldLeft;
	}
	else if ( pos_.x >= GameConst::fieldRight ) {
		pos_.x = GameConst::fieldRight;
	}
}

void Player::applyInput( std::uint8_t keys ) {
	keys &= Protocol::inputKeyMask;

	// 이전에 받은 키와 비교해서 바뀐 비트만 모음
	tapped_ |= keys & ~keys_;
	released_ |= keys_ & ~keys;
	keys_ = keys;
}

Protocol::PlayerState Player::toState( ) const {
	auto state = Protocol::PlayerState{ };
	state.x = pos_.x;
	state.y = pos_.y;

	// 살아 있는데 0으로 보이지 않도록 올림 (82.5 -> 83)
	auto hp = std::clamp( std::ceil( hp_ ), 0.f, GameConst::playerMaxHp );
	state.hp = static_cast<std::uint8_t>( hp );

	state.anim = kickAnimT_ > 0.f ? Protocol::PlayerAnim::Kick : anim_;
	state.stunned = isStunned( ) ? 1 : 0;
	return state;
}
