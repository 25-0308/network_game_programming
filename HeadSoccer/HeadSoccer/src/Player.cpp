#include "../include/Player.hpp"
#include "../include/Timer.hpp"
#include "../include/InputHandler.hpp"
#include "../include/Camera.hpp"
#include "../include/EventHandler.hpp"
#include "../include/SceneHandler.hpp"
#include "../include/Ball.hpp"
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
    kickCooldown_ = kickCooldownTime;
    startKick( );
}

void Player::startKick( ) {
    SoundHandler::playEffect( SoundEffect::PlayerKick );

    kickAnimT_ = kickFrameCnt * kickFrameTime;
    getAnimator( )->playFromStart( kickAnimName( ) );

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

    if ( isStunned( ) || damage <= 0.f ) {
        return;
    }

    hp_ = std::max( hp_ - damage, 0.f );
    SoundHandler::playHitSound( characterName_ );
    if ( hp_ > 0.f ) {
        return;
    }

    // knocked out: stand still until stunT_ runs out, then come back with full hp
    stunT_ = stunTime;
    kickAnimT_ = 0.f;
    getAnimator( )->play( idleAnimName( ) );
}

void Player::update( const Protocol::PlayerState& state ) {
    pos_ = PointFloat( state.x, state.y );
    hp_ = static_cast<float>( state.hp );
    // 남은 기절 시간은 오지 않으므로 기절 여부만 맞춤 (기절이 풀리는 시점도 서버를 따름)
    stunT_ = state.stunned ? 1.f : 0.f;

    applyServerAnim( state.anim );

    // 애니메이션은 S2C_Frame마다 update( )에서 진행하므로 여기서는 충돌 박스만 옮김
    getCollider( )->update( pos_ );
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
            kickCooldown_ = kickCooldownTime;
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
        // landed on the other player's head: stand on it
        // (점프 속도와 착지 상태는 서버 Player::separateFrom이 처리)
        Player* upper = delta.y > 0.f ? this : other;
        upper->pos_.y -= overlapY;
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
