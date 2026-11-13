#include "../include/Ball.hpp"
#include "../include/Timer.hpp"
#include "../include/Camera.hpp"
#include "../include/Player.hpp"
#include "../include/SoundHandler.hpp"

Ball::Ball(std::string name)
    : image{ ResourceHandler::loadImage(name, "assets/ball.png", true) },
      simulation_{ 0.f, 0.f, image->width() / 2.f } {
    createCollider( );
    getCollider( )->setScale( PointFloat( static_cast<float>( image->width( ) ),
        static_cast<float>( image->height( ) ) ) );
    setScale( PointFloat( image->width( ) / 2.f, image->height( ) / 2.f ) );
}

void Ball::kickleft() {
    playImpactSound( 2000.f );
    simulation_.kickleft( );
}

void Ball::kickright( ) {
    playImpactSound( 2000.f );
    simulation_.kickright( );
}

void Ball::render(HDC hdc) {
    auto relativePos = pos_ - Camera::getPos();
    if (image) {
        image->draw(hdc, relativePos);
    }
    componentRender(hdc);
}

void Ball::reset( PointFloat pos ) {
    simulation_.reset( pos );
    applyState( simulation_.getState( ) );
    impactSoundCooldown_ = 0.f;
}

void Ball::update() {
    const float dt = Timer::fdt( );
    impactSoundCooldown_ = std::max( 0.f, impactSoundCooldown_ - dt );

    simulation_.setPosition( pos_.x, pos_.y );
    const float impactSpeed = simulation_.update( dt );
    applyState( simulation_.getState( ) );
    playImpactSound( impactSpeed );
    componentUpdate( );
}

void Ball::applyState( const Protocol::S2C_Ball& state ) {
    pos_ = PointFloat( state.x, state.y );
    simulation_.setPosition( state.x, state.y );
    simulation_.setVelocity( state.vx, state.vy );
    getCollider( )->update( pos_ );
}

void Ball::playImpactSound( float impactSpeed ) {
    // 정지 접촉과 같은 충돌의 짧은 간격 반복에는 효과음을 재생하지 않는다.
    constexpr float minimumAudibleSpeed = 80.f;
    constexpr float impactInterval = 0.06f;
    if ( impactSpeed < minimumAudibleSpeed || impactSoundCooldown_ > 0.f ) {
        return;
    }
    SoundHandler::playEffect( SoundEffect::BallImpact );
    impactSoundCooldown_ = impactInterval;
}

void Ball::onCollisionEntry( Object* other ) {
    handleCollision( other, true );
}

void Ball::onCollision( Object* other ) {
    handleCollision( other, false );
}

void Ball::handleCollision( Object* other, bool entering ) {
    if ( other == nullptr || other->getCollider( ) == nullptr ) {
        return;
    }

    float minPushSpeed = 0.f;
    if ( dynamic_cast<Player*>( other ) ) {
        minPushSpeed = 300.f;
    }
    else if ( !other->getName( ).starts_with( "goalpost" ) ) {
        return;
    }

    const auto* collider = other->getCollider( );
    simulation_.setPosition( pos_.x, pos_.y );
    float impactSpeed;
    if ( entering ) {
        impactSpeed = simulation_.onCollisionEntry( collider->getFinalPos( ),
            collider->getScale( ), minPushSpeed );
    }
    else {
        impactSpeed = simulation_.onCollision( collider->getFinalPos( ),
            collider->getScale( ), minPushSpeed );
    }
    applyState( simulation_.getState( ) );
    playImpactSound( impactSpeed );
}
