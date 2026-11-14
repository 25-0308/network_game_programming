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
	anim_ = Protocol::PlayerAnim::Idle;
}

void Player::update( float dt ) {
	// 클라 Player::update와 같은 순서: 중력 -> 타이머 -> (기절이 아니면) 점프, 좌, 우 -> 벽
	applyGravity( dt );
	tickTimers( dt );

	if ( !isStunned( ) ) {
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

	// 기절 감소, 발차기 쿨타임, 모션 시간은 11/15 발차기, 피격과 같이 옮김
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
	// 클라 Player는 scale_이 (0, 0)이라 중심 x가 그대로 벽에 걸림
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
