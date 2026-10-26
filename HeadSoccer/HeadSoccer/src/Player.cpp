#include "../include/Player.hpp"
#include "../include/Timer.hpp"
#include "../include/InputHandler.hpp"
#include "../include/Camera.hpp"
#include "../include/EventHandler.hpp"
#include "../include/SceneHandler.hpp"

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

    if (pnum_==1) {
        if (InputHandler::getKeyState(Key::S) == KeyState::Tap) {
                
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
			getAnimator( )->play( "player_run_back_left" );
        }
		else if ( InputHandler::getKeyState( Key::A ) == KeyState::Away ) {
			getAnimator( )->play( "player_idle_left" );
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
			getAnimator( )->play( "player_run_front_left" );
        }
        else if ( InputHandler::getKeyState( Key::D ) == KeyState::Away ) {
            getAnimator( )->play( "player_idle_left" );
        }
    }

    if (pnum_ == 2) {
        if (InputHandler::getKeyState(Key::K) == KeyState::Tap) {

        }

        if (jumped == false && InputHandler::getKeyState(Key::I) == KeyState::Tap) {
            jspeed = jumppower;
            jumped = true;
        }

        if (InputHandler::getKeyState(Key::J) == KeyState::Tap ||
            InputHandler::getKeyState(Key::J) == KeyState::Hold) {
            pos_.x -= speed * dt;
			getAnimator( )->play( "player_run_front_right" );
        }
		else if ( InputHandler::getKeyState( Key::J ) == KeyState::Away ) {
			getAnimator( )->play( "player_idle_right" );
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
			getAnimator( )->play( "player_run_back_right" );
        }
		else if ( InputHandler::getKeyState( Key::L ) == KeyState::Away ) {
			getAnimator( )->play( "player_idle_right" );
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

void Player::render( HDC hdc ) {
    componentRender( hdc );
    
    auto relativePos = pos_ - Camera::getPos( );
    if ( image_ ) {
        image_->draw( hdc, relativePos );
    }
}

void Player::onCollisionEntry( Object* other ) {
    getCollider( )->addCollCnt( );
}

void Player::onCollisionExit( Object* other ) {
	getCollider( )->subCollCnt( );
}
