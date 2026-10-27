#include "../include/ResultScene.hpp"
#include "../include/Background.hpp"
#include "../include/Button.hpp"
#include "../include/CharacterInfo.hpp"
#include "../include/Camera.hpp"
#include "../include/Player.hpp"
#include "../include/TextLabel.hpp"

extern CharacterInfo player1;
extern CharacterInfo player2;
extern int p1GoalScore;
extern int p2GoalScore;

void ResultScene::entry( ) {
	auto bg = new Background( "bg", "select_character_background.png", false );
	bg->setName( "bg" );
	bg->setPos( PointFloat( 640.f, 360.f ) );
	addObject( ObjectType::Background, bg );

	auto resultBg = new Background( "result_bg", "select_stage.png", true );
	resultBg->setName( "result_bg" );
	resultBg->setPos( PointFloat( 640.f, 330.f ) );
	addObject( ObjectType::Background, resultBg );

	auto versus = new Background( "versus", "versus.png", true );
	versus->setName( "versus" );
	versus->setPos( PointFloat( 640.f, 330.f ) );
	addObject( ObjectType::Background, versus );

	auto p1Shadow = new Background( "p1_shadow", "shadow.png", true );
	p1Shadow->setName( "p1_shadow" );
	p1Shadow->setPos( PointFloat( 300.f, 450.f ) );
	addObject( ObjectType::Background, p1Shadow );

	auto p2Shadow = new Background( "p2_shadow", "shadow.png", true );
	p2Shadow->setName( "p2_shadow" );
	p2Shadow->setPos( PointFloat( 980.f, 450.f ) );
	addObject( ObjectType::Background, p2Shadow );

	auto p1ImageName = player1.getImage( );
	auto temp = std::string( );
	for ( auto c : p1ImageName ) {
		if ( c == '_' ) {
			break;
		}
		temp.push_back( c );
	}
	auto p1 = new Player( temp, player1.getSpeedStat( ),
		player1.getKickStat( ), player1.getJumpStat( ), "left", 1 );
	p1->setName( "player1" );
	p1->setPos( PointFloat( 300.f, 350.f ) );
	addObject( ObjectType::Player, p1 );

	auto p1ImageName2 = player2.getImage( );
	auto temp2 = std::string( );
	for ( auto c : p1ImageName2 ) {
		if ( c == '_' ) {
			break;
		}
		temp2.push_back( c );
	}
	auto p2 = new Player( temp2, player2.getSpeedStat( ),
		player2.getKickStat( ), player2.getJumpStat( ), "right", 2 );
	p2->setName( "player2" );
	p2->setPos( PointFloat( 980.f, 350.f ) );
	addObject( ObjectType::Player, p2 );

	// more goals wins; there's no draw image, so a draw is drawn as text
	if ( p1GoalScore == p2GoalScore ) {
		for ( auto x : { 300.f, 980.f } ) {
			auto draw = new TextLabel( L"DRAW", 80, RGB( 255, 255, 255 ) );
			draw->setName( "draw" );
			draw->setPos( PointFloat( x, 200.f ) );
			addObject( ObjectType::Background, draw );
		}
	}
	else {
		bool p1Won = p1GoalScore > p2GoalScore;

		auto win = new Background( "win", "win.png", true );
		win->setName( "win" );
		win->setPos( PointFloat( p1Won ? 300.f : 980.f, 200.f ) );
		addObject( ObjectType::Background, win );

		auto lose = new Background( "lose", "lose.png", true );
		lose->setName( "lose" );
		lose->setPos( PointFloat( p1Won ? 980.f : 300.f, 200.f ) );
		addObject( ObjectType::Background, lose );
	}

	Camera::setPos( { 0.f, 0.f } );
}

void ResultScene::exit( ) {
	Scene::destroyObjGroups( );
}
