#include "Object.hpp"
#include <string>
#include "Resourcehandler.hpp"

class Player : public Object
{
public:
	Player(const std::string& name, int ss, int ks, int js, const std::string& lr,int num) {
		auto filePath = fs::path( "assets/" + name + "_player_" + lr + ".png" );
		auto keyVal = name + "_player_" + lr;
		image_ = ResourceHandler::loadImage( keyVal, fs::canonical( filePath ).string( ), true );
		
		kickpower = ks * 50.f;
		jumppower =  -( 1250.f + (40.f*js));
		speed = ss * 50.f;
		jspeed = 0.0f;
		ldtimer_ = 0.0f;
		rdtimer_ = 0.0f;
		dashdis = 100.0f / 0.075f;
		dashtimer = 0.0f;
		dashintime = 0.0f;
		ldashed = false;
		rdashed = false;
		jumped = false;

		
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

			getAnimator( )->play( "player_idle_right" );
		}


		createCollider( );
		getCollider( )->setScale( PointFloat( 70.f, 90.f ) );
		getCollider( )->setOffset( PointFloat( 0.f, 15.f ) );
		
		speedstat = ss;
		kickstat = ks;
		jumpstat = js;
		pnum_ = num;
	}

	void update()override;
	void render(HDC hdc)override;

	void onCollision( Object* other ) override {}
	void onCollisionEntry( Object* other ) override;
	void onCollisionExit( Object* other ) override;

	void ispeed() {
		speed += 0.1f;
	}

	void dspeed() {
		if (speed > 0.1f) speed -= 0.1f;
	}

	void kick();

private:
	int speedstat;
	int kickstat;
	int jumpstat;
	Image* image_;
	std::string flag_;
	std::string leftorright_;
	float kickpower;
	float jumppower;
	int pnum_;
	float speed;
	float jspeed;
	float ldtimer_;
	float rdtimer_;
	float dashdis;
	float dashtimer;
	float dashintime;
	bool ldashed;
	bool rdashed;
	bool jumped;
};

