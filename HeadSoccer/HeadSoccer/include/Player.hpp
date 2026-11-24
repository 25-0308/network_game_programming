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
	// ks(발차기) 스탯은 서버 Player가 피격 데미지 계산에 씀 (클라는 S2C_GameState HP만 받음)
	Player(const std::string& name, [[maybe_unused]] int ss, [[maybe_unused]] int ks, [[maybe_unused]] int js, const std::string& lr,int num)
		: characterName_{ name } {
		auto filePath = fs::path( "assets/" + name + "_player_" + lr + ".png" );
		auto keyVal = name + "_player_" + lr;
		image_ = ResourceHandler::loadImage( keyVal, fs::canonical( filePath ).string( ), true );

		// 첫 S2C_GameState를 받기 전까지 보여 줄 값 (HP, 기절은 서버 값으로 덮음)
		hp_ = maxHp;
		stunned_ = false;
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

		pnum_ = num;
	}

	void update()override;
	// S2C_GameState로 받은 서버 상태(위치, HP, 기절, 애니메이션) 적용
	void update( const Protocol::PlayerState& state );
	void render(HDC hdc)override;

	void onCollisionEntry( Object* other ) override;
	void onCollisionExit( Object* other ) override;

	void kick();

	// HP, 기절은 서버 Player가 판정하고 S2C_GameState로 받은 값
	bool isStunned( ) const {
		return stunned_;
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
		lDashTapT_ = 0.f;
		rDashTapT_ = 0.f;
		dashT_ = 0.f;
		dashCooldownT_ = 0.f;
		lDashing_ = false;
		rDashing_ = false;
		kickCooldown_ = 0.0f;
		kickAnimT_ = 0.0f;
		getAnimator( )->play( idleAnimName( ) );
		getCollider( )->update( pos_ );
	}

private:
	const std::string characterName_;
	Image* image_;
	int pnum_;
	float hp_;
	bool stunned_;
	float kickCooldown_;
	float kickAnimT_;
	std::optional<Protocol::PlayerAnim> serverAnim_;	// 마지막으로 받은 서버 애니메이션

	// 대시 효과음 판정용 (대시 이동은 서버가 함), 서버 Player의 대시 규칙과 같음
	float lDashTapT_ = 0.f;		// 왼쪽 키 두 번째 뗌을 기다리는 시간
	float rDashTapT_ = 0.f;		// 오른쪽 키 두 번째 뗌을 기다리는 시간
	float dashT_ = 0.f;			// 대시 남은 시간
	float dashCooldownT_ = 0.f;	// 다음 대시까지 남은 시간
	bool lDashing_ = false;
	bool rDashing_ = false;

	// 한 방향 키의 대시 판정, 대시가 시작되면 효과음만 재생
	// released: 이번 프레임에 그 방향 키를 뗌 (KeyState::Away)
	void updateDashSound( bool released, float& myTapT, float& otherTapT, bool& dashing, float dt );

	// 발차기 시작: 소리, 모션, 공 판정 (쿨타임 확인은 호출하는 쪽에서)
	// 상대 피격 판정은 서버 Match::resolveKick이 함
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

