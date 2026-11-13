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
