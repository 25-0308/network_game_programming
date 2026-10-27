#include "../include/Player.hpp"
#include "../include/Timer.hpp"
#include "../include/InputHandler.hpp"
#include "../include/Camera.hpp"
#include "../include/EventHandler.hpp"
#include "../include/SceneHandler.hpp"
#include "../include/Ball.hpp"

void Player::update() {
    // Esc scene change is handled by player1 only, so the event is not queued twice
    if ( pnum_ == 1 && InputHandler::getKeyState( Key::Esc ) == KeyState::Tap
        && SceneHandler::getCurrSceneType( ) == SceneType::Result ) {
        EventHandler::addEvent(
            Event{
                .type = EventType::ChangeScene,
                .wParam = static_cast<DWORD_PTR>( SceneType::SelectCharacter ),
            }
         );
        return;
    }
    if ( SceneHandler::getCurrSceneType( ) == SceneType::Result ) {
        componentUpdate( );
		return;
    }

	if ( pnum_ == 1 && InputHandler::getKeyState( Key::Esc ) == KeyState::Tap ) {
		EventHandler::addEvent( 
            Event{
				.type = EventType::ChangeScene,
				.wParam = static_cast<DWORD_PTR>( SceneType::Result ),
            }
        );
		return;
	}

    float dt = Timer::fdt();

    float width = 1280.0f;
    float height = 580.f;

    pos_.y += 1000.0f * dt;

    pos_.y += jspeed * dt;
    jspeed += 1000.0f * dt;

    if (pos_.y + scale_.y >= height) {
        pos_.y = height - scale_.y;
        jumped = false;
    }

    ldtimer_ -= dt;
    rdtimer_ -= dt;
    dashintime -= dt;
    dashdis = 100.0f / 0.05f;

    if ( isStunned( ) ) {
        stunT_ -= dt;
        if ( stunT_ <= 0.f ) {
            stunT_ = 0.f;
            hp_ = maxHp;
        }
    }

    kickCooldown_ -= dt;
    if ( kickAnimT_ > 0.f ) {
        kickAnimT_ -= dt;
        if ( kickAnimT_ <= 0.f ) {
            getAnimator( )->play( idleAnimName( ) );
        }
    }

    if (pnum_==1 && !isStunned()) {
        if (InputHandler::getKeyState(Key::S) == KeyState::Tap) {
            kick();
        }

        if (jumped == false && InputHandler::getKeyState(Key::W) == KeyState::Tap) {
            jspeed = jumppower;
            jumped = true;
        }

        if (dashintime <= 0.0f && InputHandler::getKeyState(Key::A) == KeyState::Away && ldtimer_ <= 0.0f) {
            ldtimer_ = 0.3f;
            rdtimer_ = 0.0f;
        }
        else if (dashintime <= 0.0f && InputHandler::getKeyState(Key::A) == KeyState::Away && ldtimer_ >= 0.0f) {
            ldashed = true;
            ldtimer_ = 0.0f;
            dashtimer = 0.075f;
        }
        if (ldashed) {
            pos_.x -= dashdis * dt;
            dashtimer -= dt;
            if (dashtimer <= 0.0f) {
                dashintime = 0.5f;
                ldashed = false;
            }
        }

        if (InputHandler::getKeyState(Key::A) == KeyState::Tap ||
            InputHandler::getKeyState(Key::A) == KeyState::Hold) {
            pos_.x -= speed * dt;
			playAnim( "player_run_back_left" );
        }
		else if ( InputHandler::getKeyState( Key::A ) == KeyState::Away ) {
			playAnim( "player_idle_left" );
		}

        if (dashintime <= 0.0f && InputHandler::getKeyState(Key::D) == KeyState::Away && rdtimer_ <= 0.0f) {
            ldtimer_ = 0.0f;
            rdtimer_ = 0.3f;
        }
        else if (dashintime <= 0.0f && InputHandler::getKeyState(Key::D) == KeyState::Away && rdtimer_ >= 0.0f) {
            rdashed = true;
            dashdis = 100.0f / 0.075f;
            rdtimer_ = 0.0f;
            dashtimer = 0.075f;
        }
        if (rdashed) {
            pos_.x += dashdis * dt;
            dashtimer -= dt;
            if (dashtimer <= 0.0f) {
                dashintime = 0.5f;
                rdashed = false;
            }
        }

        if (InputHandler::getKeyState(Key::D) == KeyState::Tap ||
            InputHandler::getKeyState(Key::D) == KeyState::Hold) {
            pos_.x += speed * dt;
			playAnim( "player_run_front_left" );
        }
        else if ( InputHandler::getKeyState( Key::D ) == KeyState::Away ) {
            playAnim( "player_idle_left" );
        }
    }

    if (pnum_ == 2 && !isStunned()) {
        if (InputHandler::getKeyState(Key::K) == KeyState::Tap) {
            kick();
        }

        if (jumped == false && InputHandler::getKeyState(Key::I) == KeyState::Tap) {
            jspeed = jumppower;
            jumped = true;
        }

        if (InputHandler::getKeyState(Key::J) == KeyState::Tap ||
            InputHandler::getKeyState(Key::J) == KeyState::Hold) {
            pos_.x -= speed * dt;
			playAnim( "player_run_front_right" );
        }
		else if ( InputHandler::getKeyState( Key::J ) == KeyState::Away ) {
			playAnim( "player_idle_right" );
		}

        if (dashintime <= 0.0f && InputHandler::getKeyState(Key::J) == KeyState::Away && ldtimer_ <= 0.0f) {
            ldtimer_ = 0.3f;
            rdtimer_ = 0.0f;
        }
        else if (dashintime <= 0.0f && InputHandler::getKeyState(Key::J) == KeyState::Away && ldtimer_ >= 0.0f) {
            ldashed = true;
            ldtimer_ = 0.0f;
            dashtimer = 0.075f;
        }
        if (ldashed) {
            pos_.x -= dashdis * dt;
            dashtimer -= dt;
            if (dashtimer <= 0.0f) {
                dashintime = 0.5f;
                ldashed = false;
            }
        }

        if (InputHandler::getKeyState(Key::L) == KeyState::Tap ||
            InputHandler::getKeyState(Key::L) == KeyState::Hold) {
            pos_.x += speed * dt;
			playAnim( "player_run_back_right" );
        }
		else if ( InputHandler::getKeyState( Key::L ) == KeyState::Away ) {
			playAnim( "player_idle_right" );
		}

        if (dashintime <= 0.0f && InputHandler::getKeyState(Key::L) == KeyState::Away && rdtimer_ <= 0.0f) {
            ldtimer_ = 0.0f;
            rdtimer_ = 0.3f;
        }
        else if (dashintime <= 0.0f && InputHandler::getKeyState(Key::L) == KeyState::Away && rdtimer_ >= 0.0f) {
            rdashed = true;
            dashdis = 100.0f / 0.05f;
            rdtimer_ = 0.0f;
            dashtimer = 0.075f;
        }
        if (rdashed) {
            pos_.x += dashdis * dt;
            dashtimer -= dt;
            if (dashtimer <= 0.0f) {
                dashintime = 0.5f;
                rdashed = false;
            }
        }
    }

    if (pos_.x - scale_.x <= 0.0f) pos_.x = scale_.x;
    else if (pos_.x + scale_.x >= width) pos_.x = width - scale_.x;

    componentUpdate( );
}

void Player::kick( ) {
    const float kickCooldownTime = 0.5f;

    if ( kickCooldown_ > 0.f ) {
        return;
    }
    kickCooldown_ = kickCooldownTime;

    kickAnimT_ = kickFrameCnt * kickFrameTime;
    getAnimator( )->playFromStart( pnum_ == 1 ? "player_kick_left" : "player_kick_right" );

    const float kickRange = 90.f;
    const float kickHitReach = 40.f;
    // kick stat 5 -> 17.5 (6 hits to stun), 6 -> 21 (5 hits), 8 -> 28 (4 hits)
    const float damagePerKickStat = 3.5f;
    const float kickDamage = kickstat * damagePerKickStat;

    // player1 stands on the left and kicks right, player2 the opposite
    auto foot = pos_ + PointFloat( pnum_ == 1 ? 30.f : -30.f, 50.f );
    auto currScene = SceneHandler::getCurrScene( );

    for ( Object* obj : currScene->getGroup( ObjectType::Player ) ) {
        if ( obj == this ) {
            continue;
        }

        // distance from the foot to the nearest point of the opponent's body box
        auto body = obj->getCollider( );
        auto boxPos = body->getFinalPos( );
        auto half = body->getScale( ) / 2.f;
        auto closest = PointFloat(
            std::clamp( foot.x, boxPos.x - half.x, boxPos.x + half.x ),
            std::clamp( foot.y, boxPos.y - half.y, boxPos.y + half.y ) );
        auto diff = foot - closest;
        if ( std::hypot( diff.x, diff.y ) <= kickHitReach ) {
            static_cast<Player*>( obj )->takeHit( kickDamage );
        }
    }

    for ( Object* obj : currScene->getGroup( ObjectType::Ball ) ) {
        auto ball = static_cast<Ball*>( obj );
        auto diff = ball->getPos( ) - foot;
        if ( std::hypot( diff.x, diff.y ) > kickRange ) {
            continue;
        }

        if ( pnum_ == 1 ) {
            ball->kickright( );
        }
        else {
            ball->kickleft( );
        }
    }
}

void Player::takeHit( float damage ) {
    const float stunTime = 2.f;

    if ( isStunned( ) ) {
        return;
    }

    hp_ = std::max( hp_ - damage, 0.f );
    if ( hp_ > 0.f ) {
        return;
    }

    // knocked out: stand still until stunT_ runs out, then come back with full hp
    stunT_ = stunTime;
    ldashed = false;
    rdashed = false;
    kickAnimT_ = 0.f;
    getAnimator( )->play( idleAnimName( ) );
}

void Player::render( HDC hdc ) {
    componentRender( hdc );
    
    auto relativePos = pos_ - Camera::getPos( );
    if ( image_ ) {
        image_->draw( hdc, relativePos );
    }
}

void Player::onCollisionEntry( Object* other ) {
    getCollider( )->addCollCnt( );
    onCollision( other );
}

void Player::onCollision( Object* other ) {
    // both players get this callback, so only player1 resolves the pair
    auto otherPlayer = dynamic_cast<Player*>( other );
    if ( otherPlayer != nullptr && pnum_ == 1 ) {
        separateFrom( otherPlayer );
    }
}

void Player::separateFrom( Player* other ) {
    auto mine = getCollider( );
    auto theirs = other->getCollider( );

    auto delta = theirs->getFinalPos( ) - mine->getFinalPos( );
    float overlapX = ( mine->getScale( ).x + theirs->getScale( ).x ) / 2.f - std::abs( delta.x );
    float overlapY = ( mine->getScale( ).y + theirs->getScale( ).y ) / 2.f - std::abs( delta.y );
    if ( overlapX <= 0.f || overlapY <= 0.f ) {
        return;
    }

    if ( overlapY < overlapX ) {
        // landed on the other player's head: stand on it, and allow jumping off
        Player* upper = delta.y > 0.f ? this : other;
        upper->pos_.y -= overlapY;
        upper->jspeed = 0.f;
        upper->jumped = false;
    }
    else {
        // side by side: push both apart by half the overlap
        float dir = delta.x >= 0.f ? 1.f : -1.f;
        pos_.x -= dir * overlapX / 2.f;
        other->pos_.x += dir * overlapX / 2.f;
    }

    mine->update( pos_ );
    theirs->update( other->pos_ );
}

void Player::onCollisionExit( Object* other ) {
	getCollider( )->subCollCnt( );
}
