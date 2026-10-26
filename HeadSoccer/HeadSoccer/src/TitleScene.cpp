#include "../include/Game.hpp"
#include "../include/TitleScene.hpp"
#include "../include/Camera.hpp"
#include "../include/Background.hpp"
#include "../include/Button.hpp"

void TitleScene::entry( ) {
	auto bg = new Background( "title_bg", "title_bg.png", true );

	auto clientWidth = Game::getRsl( ).x;
	auto clientHeight = Game::getRsl( ).y;

	bg->setName( "title_bg" );
	bg->setPos( PointFloat( clientWidth / 2.f, clientHeight / 2.f ) );
	addObject( ObjectType::Background, bg );

	auto arcadeBtn = new Button( "arcade_btn", "arcade", true );
	arcadeBtn->setName( "arcade_btn" );
	arcadeBtn->setPos( PointFloat( clientWidth / 4.5f, clientHeight / 2.f ) );
	addObject( ObjectType::Button, arcadeBtn );

	Camera::setPos( { 0.f, 0.f } );
}

void TitleScene::exit( ) {
	Scene::destroyObjGroups( );
}
