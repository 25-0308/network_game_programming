#ifndef __player_hpp
#define __player_hpp

#include "Object.hpp"
#include <string>
#include "Resourcehandler.hpp"
#include "../../../Common/Protocol.hpp"
#include <optional>

class Player : public Object
{
public:
	// ss(속도), js(점프) 스탯은 서버 Player가 이동 계산에 씀 (클라는 S2C_GameState 위치만 받음)
	Player(const std::string& name, [[maybe_unused]] int ss, int ks, [[maybe_unused]] int js, const std::string& lr,int num)
		: characterName_{ name } {
		auto filePath = fs::path( "assets/" + name + "_player_" + lr + ".png" );
		auto keyVal = name + "_player_" + lr;
		image_ = ResourceHandler::loadImage( keyVal, fs::canonical( filePath ).string( ), true );

		hp_ = maxHp;
		stunT_ = 0.0f;
		kickCooldown_ = 0.0f;
		kickAnimT_ = 0.0f;


		createAnimator( );
		if ( lr == "left" ) {
			auto idleAnimFilePath = fs::path( "assets/player_idle_left.png" );
			auto idleAnimImage = ResourceHandler::loadImage( "player_idle_left", fs::canonical( idleAnimFilePath ).string( ), true );

			getAnimator( )->createAnimation( "player_idle_left", idleAnimImage, PointFloat( 0.f, 0.f ),
				PointFloat( 93.f, 33.f ), PointFloat( 93.f, 0.f ), 6, 0.07f, PointFloat( 0.f, 53.f ) );

			auto runAnimFilePath = fs::path( "assets/player_run_left.png" );
			auto runAnimImage = ResourceHandler::loadImage( "player_run_left", fs::canonical( runAnimFilePath ).string( ), true );

			getAnimator( )->createAnimation( "player_run_front_left", runAnimImage, PointFloat( 522.5f, 0.f ),
				PointFloat( 104.5f, 30.f ), PointFloat( -104.5f, 0.f ), 6, 0.07f, PointFloat( 0.f, 53.f ) );
			getAnimator( )->createAnimation( "player_run_back_left", runAnimImage, PointFloat( 0.f, 0.f ),
				PointFloat( 104.5f, 30.f ), PointFloat( 104.5f, 0.f ), 6, 0.07f, PointFloat( 0.f, 53.f ) );

			auto kickAnimFilePath = fs::path( "assets/player_kick_left.png" );
			auto kickAnimImage = ResourceHandler::loadImage( "player_kick_left", fs::canonical( kickAnimFilePath ).string( ), true );

			getAnimator( )->createAnimation( "player_kick_left", kickAnimImage, PointFloat( 0.f, 0.f ),
				PointFloat( 123.f, 61.f ), PointFloat( 123.f, 0.f ), kickFrameCnt, kickFrameTime, PointFloat( 0.f, 45.f ) );

			getAnimator( )->play( "player_idle_left" );
		}
		else if ( lr == "right" ) {
			auto idleAnimFilePath = fs::path( "assets/player_idle_right.png" );
			auto idleAnimImage = ResourceHandler::loadImage( "player_idle_right", fs::canonical( idleAnimFilePath ).string( ), true );

			getAnimator( )->createAnimation( "player_idle_right", idleAnimImage, PointFloat( 0.f, 0.f ),
				PointFloat( 93.f, 33.f ), PointFloat( 93.f, 0.f ), 6, 0.07f, PointFloat( 0.f, 53.f ) );

			auto runAnimFilePath = fs::path( "assets/player_run_right.png" );
			auto runAnimImage = ResourceHandler::loadImage( "player_run_right", fs::canonical( runAnimFilePath ).string( ), true );
			
			getAnimator( )->createAnimation( "player_run_front_right", runAnimImage, PointFloat( 0.f, 0.f ),
				PointFloat( 104.5f, 30.f ), PointFloat( 104.5f, 0.f ), 6, 0.07f, PointFloat( 0.f, 53.f ) );
			getAnimator( )->createAnimation( "player_run_back_right", runAnimImage, PointFloat( 522.5f, 0.f ),
				PointFloat( 104.5f, 30.f ), PointFloat( -104.5f, 0.f ), 6, 0.07f, PointFloat( 0.f, 53.f ) );

			auto kickAnimFilePath = fs::path( "assets/player_kick_right.png" );
			auto kickAnimImage = ResourceHandler::loadImage( "player_kick_right", fs::canonical( kickAnimFilePath ).string( ), true );

			getAnimator( )->createAnimation( "player_kick_right", kickAnimImage, PointFloat( 0.f, 0.f ),
				PointFloat( 123.f, 61.f ), PointFloat( 123.f, 0.f ), kickFrameCnt, kickFrameTime, PointFloat( 0.f, 45.f ) );

			getAnimator( )->play( "player_idle_right" );
		}


		createCollider( );
		getCollider( )->setScale( PointFloat( 70.f, 90.f ) );
		getCollider( )->setOffset( PointFloat( 0.f, 15.f ) );
		
		kickstat = ks;
		pnum_ = num;
	}

	void update()override;
	// S2C_GameState로 받은 서버 상태(위치, HP, 기절, 애니메이션) 적용
	void update( const Protocol::PlayerState& state );
	void render(HDC hdc)override;

	void onCollision( Object* other ) override;
	void onCollisionEntry( Object* other ) override;
	void onCollisionExit( Object* other ) override;

	void kick();
	void takeHit( float damage );

	bool isStunned( ) const {
		return stunT_ > 0.f;
	}
	float getHp( ) const {
		return hp_;
	}
	// 1 = left (1P), 2 = right (2P), S2C_GameState::players[ num - 1 ]
	int getPlayerNum( ) const {
		return pnum_;
	}

	// 원격 Player 모드: 이 클라이언트가 조작하지 않는 상대 Player
	// 키 입력과 이동 계산 없이 서버 상태만 보여 줌
	bool isRemote( ) const;

	static constexpr float maxHp = 100.f;
	static constexpr float kickCooldownTime = 0.5f;

	// player_kick_*.png has 8 frames of 123x61
	static constexpr std::uint8_t kickFrameCnt = 8;
	static constexpr float kickFrameTime = 0.03f;

	// back to the kickoff position with no kick in progress
	// (점프, 대시 상태는 서버 Player::resetToKickoff가 초기화)
	void reset( PointFloat pos ) {
		pos_ = pos;
		kickCooldown_ = 0.0f;
		kickAnimT_ = 0.0f;
		getAnimator( )->play( idleAnimName( ) );
		getCollider( )->update( pos_ );
	}

private:
	const std::string characterName_;
	int kickstat;
	Image* image_;
	int pnum_;
	float hp_;
	float stunT_;
	float kickCooldown_;
	float kickAnimT_;
	std::optional<Protocol::PlayerAnim> serverAnim_;	// 마지막으로 받은 서버 애니메이션

	void separateFrom( Player* other );

	// 발차기 시작: 소리, 모션, 상대와 공 판정 (쿨타임 확인은 호출하는 쪽에서)
	void startKick( );

	// 서버 애니메이션 적용, Kick으로 바뀌는 순간을 발차기 이벤트로 봄
	void applyServerAnim( Protocol::PlayerAnim anim );

	// movement animations wait until the kick motion has finished
	void playAnim( const std::string& name ) {
		if ( kickAnimT_ <= 0.f ) {
			getAnimator( )->play( name );
		}
	}
	const char* idleAnimName( ) const {
		return pnum_ == 1 ? "player_idle_left" : "player_idle_right";
	}
	const char* kickAnimName( ) const {
		return pnum_ == 1 ? "player_kick_left" : "player_kick_right";
	}
	// Idle, RunFront, RunBack의 애니메이션 이름 (Kick은 kickAnimName)
	const char* moveAnimName( Protocol::PlayerAnim anim ) const {
		const bool left = pnum_ == 1;
		switch ( anim ) {
		case Protocol::PlayerAnim::RunFront:
			return left ? "player_run_front_left" : "player_run_front_right";
		case Protocol::PlayerAnim::RunBack:
			return left ? "player_run_back_left" : "player_run_back_right";
		default:
			return idleAnimName( );
		}
	}
};

#endif	// __player_hpp

