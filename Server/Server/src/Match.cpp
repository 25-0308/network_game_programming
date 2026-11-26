#include "../include/Match.hpp"

#include "../include/Timer.hpp"

void Match::start( Protocol::Character p1Character, Protocol::Character p2Character ) {
	players_[ 0 ].init( 1, p1Character );
	players_[ 1 ].init( 2, p2Character );
	ball_.reset( PointFloat( GameConst::ballStartX, GameConst::ballStartY ) );

	// 지난 경기가 골 화면 중에 끝났거나 중단됐어도 새로 시작
	celebrating_ = false;
	celebrateT_ = 0.f;
	scorer_ = 0;

	Timer::startMatch( );
}

void Match::applyInput( std::uint8_t playerNum, std::uint8_t keys ) {
	if ( playerNum < 1 || playerNum > Protocol::maxPlayers ) {
		return;
	}
	players_[ playerNum - 1 ].applyInput( keys );
}

void Match::tick( ) {
	constexpr float dt = GameConst::tickDt;

	// 클라 순서와 같게: 1P update(발차기 포함) -> 2P update -> 충돌(밀어내기)
	// 1P 발차기로 2P가 기절하면 2P는 같은 틱에 발차기를 하지 못함
	for ( auto i = 0; i < Protocol::maxPlayers; ++i ) {
		auto& me = players_[ i ];
		auto& other = players_[ 1 - i ];

		me.update( dt );
		if ( me.justKicked( ) ) {
			resolveKick( me, other );
		}
	}

	ball_.update( dt );

	// 클라는 공, 플레이어 다음에 GoalManager를 갱신하고 그 뒤에 충돌을 처리했음
	updateGoal( dt );

	// 클라는 CollisionHandler가 겹침을 찾고 1P의 onCollision에서만 밀어냈음
	players_[ 0 ].separateFrom( players_[ 1 ] );

	for ( const auto& player : players_ ) {
		auto body = player.body( );
		ball_.onCollision( PointFloat( body.center.x, body.center.y ),
			PointFloat( body.size.x, body.size.y ), GameConst::ballPlayerMinPush );
	}

	const PointFloat crossbarSize( GameConst::crossbarWidth, GameConst::crossbarHeight );
	ball_.onCollision( PointFloat( GameConst::leftCrossbarX, GameConst::crossbarY ),
		crossbarSize, GameConst::ballCrossbarMinPush );
	ball_.onCollision( PointFloat( GameConst::rightCrossbarX, GameConst::crossbarY ),
		crossbarSize, GameConst::ballCrossbarMinPush );
}

void Match::resolveKick( Player& kicker, Player& target ) {
	if ( kicker.kickHits( target ) ) {
		target.takeHit( kicker.kickDamage( ) );
	}

	auto state = ball_.getState( );
	if ( kicker.kickReachesBall( Vec2( state.x, state.y ) ) ) {
		if ( kicker.playerNum( ) == 1 ) {
			ball_.kickright( );
		}
		else {
			ball_.kickleft( );
		}
	}
}

void Match::updateGoal( float dt ) {
	// 골 화면 중에도 경기는 진행되지만 킥오프 전까지 새 골은 세지 않음
	if ( celebrating_ ) {
		celebrateT_ += dt;
		if ( celebrateT_ >= GameConst::goalScreenTime ) {
			kickoff( );
		}
		return;
	}

	// 공 중심이 골라인을 넘고 크로스바 아래에 있으면 골
	auto state = ball_.getState( );
	if ( state.y < GameConst::crossbarBottom ) {
		return;
	}

	// 2P는 왼쪽 골대, 1P는 오른쪽 골대를 공격
	if ( state.x <= GameConst::leftGoalLine ) {
		scorer_ = 2;
	}
	else if ( state.x >= GameConst::rightGoalLine ) {
		scorer_ = 1;
	}
	else {
		return;
	}

	celebrating_ = true;
	celebrateT_ = 0.f;
}

void Match::kickoff( ) {
	ball_.reset( PointFloat( GameConst::ballStartX, GameConst::ballStartY ) );

	// HP, 기절은 그대로 두고 위치와 점프, 대시, 발차기 상태만 되돌림 (클라 Player::reset과 같음)
	for ( auto& player : players_ ) {
		player.resetToKickoff( );
	}

	celebrating_ = false;
}

std::uint8_t Match::takeScorer( ) {
	auto scorer = scorer_;
	scorer_ = 0;
	return scorer;
}
