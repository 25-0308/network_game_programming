#include "../include/Game.hpp"
#include "../include/StageScene.hpp"
#include "../include/Camera.hpp"
#include "../include/Background.hpp"
#include "../include/Button.hpp"

#include <iostream>

StageNumber stageNum = StageNumber::None;

void StageScene::entry( ) {
	auto clientWidth = Game::getRsl( ).x;
	auto clientHeight = Game::getRsl( ).y;

	auto ssbg = new Background( "select_stage_bg", "select_character_background.png", false );
	ssbg->setName( "select_stage_bg" );
	ssbg->setPos( PointFloat( clientWidth / 2.f, clientHeight / 2.f ) );
	addObject( ObjectType::Background, ssbg );

	auto ss = new Background( "select_stage", "select_stage.png", true );
	ss->setName( "select_stage" );
	ss->setPos( PointFloat( clientWidth / 2.f, 330.f ) );
	addObject( ObjectType::Background, ss );

	auto backBtn = new Button( "stage_back_button", "back_button", true );
	backBtn->setName( "stage_back_button" );
	backBtn->setPos( PointFloat( 150.f, 665.f ) );
	addObject( ObjectType::Button, backBtn );

	auto playBtn = new Button( "stage_play_button", "play_button", true );
	playBtn->setName( "stage_play_button" );
	playBtn->setPos( PointFloat( 1130.f, 665.f ) );
	addObject( ObjectType::Button, playBtn );

	auto stage1 = new Button( "mini_stage1", "mini_stage1", true );
	stage1->setName( "mini_stage1" );
	stage1->setPos( PointFloat( 400.f, 200.f ) );
	addObject( ObjectType::Button, stage1 );

	auto stage2 = new Button( "mini_stage2", "mini_stage2", true );
	stage2->setName( "mini_stage2" );
	stage2->setPos( PointFloat( 880.f, 200.f ) );
	addObject( ObjectType::Button, stage2 );

	auto stage3 = new Button( "mini_stage3", "mini_stage3", true );
	stage3->setName( "mini_stage3" );
	stage3->setPos( PointFloat( 400.f, 460.f ) );
	addObject( ObjectType::Button, stage3 );

	auto stage4 = new Button( "mini_stage4", "mini_stage4", true );
	stage4->setName( "mini_stage4" );
	stage4->setPos( PointFloat( 880.f, 460.f ) );
	addObject( ObjectType::Button, stage4 );

	Camera::setPos( { 0.f, 0.f } );
}

void StageScene::exit( ) {
	Scene::destroyObjGroups( );
}
