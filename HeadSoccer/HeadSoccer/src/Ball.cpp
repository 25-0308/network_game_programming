#include "../include/Ball.hpp"
#include "../include/Timer.hpp"
#include "../include/Camera.hpp"
#include "../include/SoundHandler.hpp"

#include <cmath>

Ball::Ball(std::string name)
    : image{ ResourceHandler::loadImage(name, "assets/ball.png", true) } {}

void Ball::render(HDC hdc) {
    auto relativePos = pos_ - Camera::getPos();
    if (image) {
        image->draw(hdc, relativePos);
    }
    componentRender(hdc);
}

void Ball::update() {
    const float dt = Timer::fdt( );
    impactSoundCooldown_ = std::max( 0.f, impactSoundCooldown_ - dt );
    componentUpdate( );
}

void Ball::update( const Protocol::S2C_Ball& state ) {
    const float impactSpeed = receivedState_
        ? std::hypot( state.vx - velo_.x, state.vy - velo_.y ) : 0.f;
    applyState( state );
    playImpactSound( impactSpeed );
    componentUpdate( );
}

void Ball::applyState( const Protocol::S2C_Ball& state ) {
    pos_ = PointFloat( state.x, state.y );
    velo_ = PointFloat( state.vx, state.vy );
    receivedState_ = true;
}

void Ball::playImpactSound( float impactSpeed ) {
    // 바닥 접촉과 작은 충돌이 짧은 간격으로 반복돼도 효과음이 겹치지 않도록 한다.
    constexpr float minimumAudibleSpeed = 80.f;
    constexpr float impactInterval = 0.06f;
    if ( impactSpeed < minimumAudibleSpeed || impactSoundCooldown_ > 0.f ) {
        return;
    }
    SoundHandler::playEffect( SoundEffect::BallImpact );
    impactSoundCooldown_ = impactInterval;
}
