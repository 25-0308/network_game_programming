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
    pos_ = pos;
    simulation_.setPosition( pos.x, pos.y );
    simulation_.setVelocity( 1.f, -100.f );
    impactSoundCooldown_ = 0.f;
    getCollider( )->update( pos_ );
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
    onCollision( other );
}

void Ball::onCollision( Object* other ) {
    if ( dynamic_cast<Player*>( other ) ) {
        // minimum push so a ball resting against a moving player still gets pushed
        bounceOffBox( other->getCollider( ), 300.f );
    }
    else if ( other->getName( ).starts_with( "goalpost" ) ) {
        // crossbar doesn't move, so the ball can rest or roll on it
        bounceOffBox( other->getCollider( ), 0.f );
    }
}

// Treat the ball as a circle and the other collider as a box: push the ball out of the box
// and reflect the velocity along the contact normal.
void Ball::bounceOffBox( const Collider* box, float minPushSpeed ) {
    const auto state = simulation_.getState( );
    auto velocity = PointFloat( state.vx, state.vy );
    const float radius = getCollider( )->getScale( ).x / 2.f;
    const float restitution = 0.8f;

    auto boxPos = box->getFinalPos( );
    auto half = box->getScale( ) / 2.f;
    float left = boxPos.x - half.x;
    float right = boxPos.x + half.x;
    float top = boxPos.y - half.y;
    float bottom = boxPos.y + half.y;

    auto closest = PointFloat( std::clamp( pos_.x, left, right ), std::clamp( pos_.y, top, bottom ) );
    auto normal = pos_ - closest;
    float dist = std::hypot( normal.x, normal.y );

    if ( dist > 0.f ) {
        // AABB overlap only at a corner of the box, the circle itself is not touching
        if ( dist >= radius ) {
            return;
        }
        normal = normal / dist;
        pos_ += normal * ( radius - dist );
    }
    else {
        // ball center is inside the box: leave through the nearest side
        float toLeft = pos_.x - left;
        float toRight = right - pos_.x;
        float toTop = pos_.y - top;
        float toBottom = bottom - pos_.y;
        float nearest = std::min( { toLeft, toRight, toTop, toBottom } );

        if ( nearest == toTop ) {
            normal = PointFloat( 0.f, -1.f );
            pos_.y = top - radius;
        }
        else if ( nearest == toLeft ) {
            normal = PointFloat( -1.f, 0.f );
            pos_.x = left - radius;
        }
        else if ( nearest == toRight ) {
            normal = PointFloat( 1.f, 0.f );
            pos_.x = right + radius;
        }
        else {
            normal = PointFloat( 0.f, 1.f );
            pos_.y = bottom + radius;
        }
    }

    float vn = velocity.x * normal.x + velocity.y * normal.y;
    if ( vn < 0.f ) {
        playImpactSound( -vn );
        velocity -= normal * ( vn * ( 1.f + restitution ) );
        vn = -vn * restitution;
    }
    if ( vn < minPushSpeed ) {
        velocity += normal * ( minPushSpeed - vn );
    }

    simulation_.setPosition( pos_.x, pos_.y );
    simulation_.setVelocity( velocity.x, velocity.y );
    getCollider( )->update( pos_ );
}
