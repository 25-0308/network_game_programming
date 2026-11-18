#include "../include/Match.hpp"

#include "../include/Timer.hpp"

void Match::start( Protocol::Character p1Character, Protocol::Character p2Character ) {
	players_[ 0 ].init( 1, p1Character );
	players_[ 1 ].init( 2, p2Character );
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

	// 클라는 CollisionHandler가 겹침을 찾고 1P의 onCollision에서만 밀어냈음
	players_[ 0 ].separateFrom( players_[ 1 ] );
}

void Match::resolveKick( Player& kicker, Player& target ) {
	if ( kicker.kickHits( target ) ) {
		target.takeHit( kicker.kickDamage( ) );
	}
}
