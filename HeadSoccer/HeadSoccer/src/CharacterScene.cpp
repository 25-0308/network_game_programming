#include "../include/Game.hpp"
#include "../include/CharacterScene.hpp"
#include "../include/Camera.hpp"
#include "../include/Background.hpp"
#include "../include/Button.hpp"
#include "../include/ImageStorage.hpp"
#include "../include/CharacterInfo.hpp"
#include "../include/TextLabel.hpp"
#include "../include/ClientNetwork.hpp"

#include <list>

CharacterInfo player1;
CharacterInfo player2;

void CharacterScene::entry( ) {
	auto clientWidth = Game::getRsl( ).x;
	auto clientHeight = Game::getRsl( ).y;
	auto playerNum = ClientNetwork::getPlayerNum( );

	auto scbg = new Background( "select_character_bg", "select_character_background.png", false );
	scbg->setName( "select_character_bg" );
	scbg->setPos( PointFloat( clientWidth / 2.f, clientHeight / 2.f ) );
	addObject( ObjectType::Background, scbg );

	auto ps = new Background( "player_select_phrase", "player_select_phrase.png", true );
	ps->setName( "player_select_phrase" );
	ps->setPos( PointFloat( clientWidth / 2.f, 55.f ) );
	addObject( ObjectType::Background, ps );

	auto sc = new Background( "select_character", "select_character.png", true );
	sc->setName( "select_character" );
	sc->setPos( PointFloat( clientWidth / 2.f, clientHeight / 2.f ) );
	addObject( ObjectType::Background, sc );

	if ( playerNum == 1 ) {
		auto lub = new Button( "left_up_button", "left_up_button", true );
		lub->setName( "left_up_button" );
		lub->setPos( PointFloat( 300.f, 180.f ) );
		addObject( ObjectType::Button, lub );

		auto ldb = new Button( "left_down_button", "left_down_button", true );
		ldb->setName( "left_down_button" );
		ldb->setPos( PointFloat( 300.f, 530.f ) );
		addObject( ObjectType::Button, ldb );
	}

	if ( playerNum == 2 ) {
		auto rub = new Button( "right_up_button", "right_up_button", true );
		rub->setName( "right_up_button" );
		rub->setPos( PointFloat( 980.f, 180.f ) );
		addObject( ObjectType::Button, rub );

		auto rdb = new Button( "right_down_button", "right_down_button", true );
		rdb->setName( "right_down_button" );
		rdb->setPos( PointFloat( 980.f, 530.f ) );
		addObject( ObjectType::Button, rdb );
	}

	auto statUi = new Background( "player1_stat_ui", "stat.png", true );
	statUi->setName( "stat_ui" );
	statUi->setPos( PointFloat( 480.f, 670.f ) );
	addObject( ObjectType::Background, statUi );

	statUi = new Background( "player2_stat_ui", "stat.png", true );
	statUi->setName( "stat_ui2" );
	statUi->setPos( PointFloat( 800.f, 670.f ) );
	addObject( ObjectType::Background, statUi );

	auto imageStorageLeft = new ImageStorage("left");
	imageStorageLeft->setName( "image_storage_left" );
	imageStorageLeft->setimagepos(PointFloat(310.f, 350.f), PointFloat(590.f, 192.5f), PointFloat(480.f, 670.f));
	addObject(ObjectType::ImageStorage, imageStorageLeft );

	auto imageStorageRight = new ImageStorage("right");
	imageStorageRight->setName( "image_storage_right" );
	imageStorageRight->setimagepos(PointFloat(985.f, 350.f), PointFloat(690.f, 192.5f), PointFloat(800.f, 670.f));
	addObject(ObjectType::ImageStorage, imageStorageRight);

	auto backBtn = new Button( "character_back_button", "back_button", true );
	backBtn->setName( "character_back_button" );
	backBtn->setPos( PointFloat( 150.f, 665.f ) );
	addObject( ObjectType::Button, backBtn );

	auto nextBtn = new Button( "character_next_button", "next_button", true );
	nextBtn->setName( "character_next_button" );
	nextBtn->setPos( PointFloat( 1130.f, 665.f ) );
	addObject( ObjectType::Button, nextBtn );

	for ( int num = 1; num <= 2; ++num ) {
		auto text = std::wstring( num == 1 ? L"1P " : L"2P " );
		if ( num == playerNum ) {
			text += L"(YOU) ";
		}
		text += L"SELECTING";
		auto color = num == playerNum ? RGB( 255, 225, 80 ) : RGB( 255, 255, 255 );
		auto label = new TextLabel( text, 24, color );
		label->setName( num == 1 ? "player1_selection_state" : "player2_selection_state" );
		label->setPos( PointFloat( num == 1 ? 310.f : 985.f, 580.f ) );
		addObject( ObjectType::Overlay, label );
	}

	Camera::setPos( { 0.f, 0.f } );
	Game::restoreSelection( );
}

void CharacterScene::exit( ) {
	Scene::destroyObjGroups( );
}
