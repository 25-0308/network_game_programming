#include "../include/Ball.hpp"
#include "../include/Timer.hpp"
#include "../include/Camera.hpp"
#include "../include/InputHandler.hpp"
#include "../include/Player.hpp"

void Ball::kickleft() {
    velo_.x -= 2000.0f;
    velo_.y -= 1000.0f;
}

void Ball::kickright( ) {
    velo_.x += 2000.0f;
    velo_.y -= 1000.0f;
}

void Ball::render(HDC hdc) {
    auto relativePos = pos_ - Camera::getPos();
    if (image) {
        image->draw(hdc, relativePos);
    }
    componentRender(hdc);
}

void Ball::update() {
	if ( InputHandler::getKeyState( Key::R ) == KeyState::Tap ) {
        kickright( );
	}
	if ( InputHandler::getKeyState( Key::Q ) == KeyState::Tap ) {
		kickleft( );
	}

    float dt = Timer::fdt();
    static float timer = 0;
    timer += dt;
    if (timer >= 1.0f) { // 좌우 속도 감소
        velo_.x = velo_.x * 0.7;
        timer = 0.0f;
    }
    velo_.y += 1000.0f * dt; // 중력
    pos_ += velo_ * dt;

    float fieldWidth = 1280.0f;
    float fieldHeight = 615.5f;

    if (pos_.x - scale_.x <= 0.0f) { // 왼벽
        velo_.x = -velo_.x;
        pos_.x = 0.0f + scale_.x;
    }
    else if (pos_.x + scale_.x >= fieldWidth) { // 오른벽
        velo_.x = -velo_.x;
        pos_.x = fieldWidth - scale_.x;
    }

    if (pos_.y - scale_.y <= 10.0f) { // 천장 충돌
        velo_.y = -velo_.y;
        pos_.y = 20.0f + scale_.y;
    }
    else if (pos_.y + scale_.y >= fieldHeight) { //지면 충돌
        velo_.y = -velo_.y * 0.8f;
        pos_.y = fieldHeight - scale_.y;
    }

    componentUpdate( );
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

    float vn = velo_.x * normal.x + velo_.y * normal.y;
    if ( vn < 0.f ) {
        velo_ -= normal * ( vn * ( 1.f + restitution ) );
        vn = -vn * restitution;
    }
    if ( vn < minPushSpeed ) {
        velo_ += normal * ( minPushSpeed - vn );
    }

    getCollider( )->update( pos_ );
}
