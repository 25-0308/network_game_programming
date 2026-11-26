#include "../include/Player.hpp"
#include "../include/Timer.hpp"
#include "../include/InputHandler.hpp"
#include "../include/Camera.hpp"
#include "../include/EventHandler.hpp"
#include "../include/SceneHandler.hpp"
#include "../include/SoundHandler.hpp"
#include "../include/ClientNetwork.hpp"
#include "GameConstants.hpp"

bool Player::isRemote( ) const {
    return pnum_ != ClientNetwork::getPlayerNum( );
}

void Player::update() {
    // 이 클라이언트가 조작하는 Player만 키를 읽음 (상대 Player는 키에 반응하지 않음)
    const bool mine = !isRemote( );

    // Esc 씬 전환은 내 Player 하나만 처리해서 이벤트가 두 번 쌓이지 않게 함
    if ( mine && InputHandler::getKeyState( Key::Esc ) == KeyState::Tap
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

	if ( mine && InputHandler::getKeyState( Key::Esc ) == KeyState::Tap ) {
		EventHandler::addEvent( 
            Event{
				.type = EventType::ChangeScene,
				.wParam = static_cast<DWORD_PTR>( SceneType::Result ),
            }
        );
		return;
	}

    // 원격 Player 모드: 키 입력, 중력, 대시, 타이머를 계산하지 않음
    // 위치, HP, 기절, 애니메이션은 S2C_GameState를 받을 때 update( state )에서 적용
    if ( !mine ) {
        componentUpdate( );
        return;
    }

    // 중력, 지면 처리, 점프(jspeed), 대시, 걷기 속도, 벽 처리는 서버 Player가 계산함
    // 위치는 S2C_GameState를 받을 때 update( state )에서 적용하고
    // 달리기, 대기 애니메이션도 서버 anim을 applyServerAnim에서 따름
    float dt = Timer::fdt();

    // 대시 효과음 판정 타이머 (서버처럼 기절 중에도 줄어듦)
    lDashTapT_ -= dt;
    rDashTapT_ -= dt;
    dashCooldownT_ -= dt;

    // 기절 시간과 기절 후 HP 회복은 서버가 계산함 (S2C_GameState의 hp, stunned)

    kickCooldown_ -= dt;
    if ( kickAnimT_ > 0.f ) {
        kickAnimT_ -= dt;
        if ( kickAnimT_ <= 0.f ) {
            getAnimator( )->play( idleAnimName( ) );
        }
    }

    if ( isStunned( ) ) {
        // 서버도 기절하면 진행 중인 대시를 끊음 (쿨타임은 걸지 않음)
        lDashing_ = false;
        rDashing_ = false;
    }
    else {
        // 발차기 모션은 키를 누른 프레임에 바로 시작 (서버 Kick 애니메이션을 기다리면 늦게 보임)
        // 1P, 2P 모두 S 발차기, A 왼쪽, D 오른쪽 (C2S_Input으로 보내는 키와 같음)
        if ( InputHandler::getKeyState( Key::S ) == KeyState::Tap ) {
            kick( );
        }

        // 대시 이동은 서버가 하고, 클라는 대시가 시작되는 순간 효과음만 냄
        updateDashSound( InputHandler::getKeyState( Key::A ) == KeyState::Away,
            lDashTapT_, rDashTapT_, lDashing_, dt );
        updateDashSound( InputHandler::getKeyState( Key::D ) == KeyState::Away,
            rDashTapT_, lDashTapT_, rDashing_, dt );
    }

    componentUpdate( );
}

void Player::updateDashSound( bool released, float& myTapT, float& otherTapT, bool& dashing, float dt ) {
    // 같은 방향 키를 dashTapWindow 안에 두 번 떼면 대시 (서버 Player::moveHorizontal과 같은 규칙)
    if ( dashCooldownT_ <= 0.f && released ) {
        if ( myTapT <= 0.f ) {
            // 첫 번째 뗌, 반대쪽 대기는 취소
            myTapT = GameConst::dashTapWindow;
            otherTapT = 0.f;
        }
        else {
            dashing = true;
            myTapT = 0.f;
            dashT_ = GameConst::dashTime;
            SoundHandler::playEffect( SoundEffect::Dash );
        }
    }

    // 대시가 끝나야 쿨타임이 시작되므로 대시 시간도 따라감
    if ( dashing ) {
        dashT_ -= dt;
        if ( dashT_ <= 0.f ) {
            dashCooldownT_ = GameConst::dashCooldown;
            dashing = false;
        }
    }
}

void Player::kick( ) {
    if ( kickCooldown_ > 0.f ) {
        return;
    }
    kickCooldown_ = GameConst::kickCooldown;
    startKick( );
}

void Player::startKick( ) {
    SoundHandler::playEffect( SoundEffect::PlayerKick );

    kickAnimT_ = GameConst::kickAnimTime;
    getAnimator( )->playFromStart( kickAnimName( ) );

    // 상대 피격(HP 감소, 기절)과 공 차기는 서버가 판정해서 S2C_GameState, S2C_Ball로 옴
}

void Player::update( const Protocol::PlayerState& state ) {
    pos_ = PointFloat( state.x, state.y );

    // HP, 기절은 서버 Player::takeHit이 판정한 값을 그대로 씀
    // 서버 HP는 맞을 때만 줄고(기절이 풀리면 다시 가득 참) 한 대에 1 이상 줄어서
    // 받은 HP가 줄어든 순간 = 맞은 순간이라 그때 피격음을 냄
    const float hp = static_cast<float>( state.hp );
    if ( hp < hp_ ) {
        SoundHandler::playHitSound( characterName_ );
    }
    hp_ = hp;

    // 기절이 시작되면 진행 중인 발차기 모션을 끊음 (서버 takeHit과 같음)
    const bool stunned = state.stunned != 0;
    if ( stunned && !stunned_ ) {
        kickAnimT_ = 0.f;
        getAnimator( )->play( idleAnimName( ) );
    }
    stunned_ = stunned;

    applyServerAnim( state.anim );

}

void Player::applyServerAnim( Protocol::PlayerAnim anim ) {
    // 서버는 발차기 모션(0.24초)보다 쿨타임(0.5초)이 길어서
    // 연속 발차기 사이에도 Kick이 아닌 상태가 끼므로 바뀌는 순간 = 발차기 한 번
    const bool kickStarted = anim == Protocol::PlayerAnim::Kick
        && serverAnim_ != Protocol::PlayerAnim::Kick;
    serverAnim_ = anim;

    if ( isRemote( ) ) {
        // 상대 Player는 서버 애니메이션을 그대로 따름
        if ( kickStarted ) {
            // 쿨타임은 서버가 판정했으므로 바로 시작
            startKick( );
        }
        else if ( anim != Protocol::PlayerAnim::Kick ) {
            kickAnimT_ = 0.f;
            getAnimator( )->play( moveAnimName( anim ) );
        }
        return;
    }

    // 내 Player는 키를 누른 프레임에 kick( )으로 먼저 모션을 시작함
    if ( kickStarted ) {
        // 쿨타임이 남아 있으면 이미 직접 찬 발차기, 없으면 내 쪽에서 놓친 발차기라 지금 시작
        if ( kickCooldown_ <= 0.f ) {
            kickCooldown_ = GameConst::kickCooldown;
            startKick( );
        }
    }
    else if ( anim != Protocol::PlayerAnim::Kick ) {
        // 서버 응답이 늦게 와도 먼저 시작한 발차기 모션은 끊지 않음
        playAnim( moveAnimName( anim ) );
    }
}

void Player::render( HDC hdc ) {
    componentRender( hdc );
    
    auto relativePos = pos_ - Camera::getPos( );
    if ( image_ ) {
        image_->draw( hdc, relativePos );
    }
}
