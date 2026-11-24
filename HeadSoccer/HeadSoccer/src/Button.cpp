#include "../include/Button.hpp"
#include "../include/PointFloat.hpp"
#include "../include/ResourceHandler.hpp"
#include "../include/EventHandler.hpp"
#include "../include/InputHandler.hpp"
#include "../include/Camera.hpp"
#include "../include/SceneHandler.hpp"
#include "../include/CharacterInfo.hpp"
#include "../include/ImageStorage.hpp"
#include "../include/Game.hpp"

//#include <iostream>
extern StageNumber stageNum;

extern CharacterInfo player1;
extern CharacterInfo player2;

Button::Button( const std::string& key, const std::string& fileName, bool premultipliedAlpha )
	: bMouseOn_{ false }, bLbtnDown_{ false }
{
	auto offBtnFilePath = fs::path( "assets/" + fileName + "_off.png" );
	auto onBtnFilePath = fs::path( "assets/" + fileName + "_on.png" );

	imageOff_ = ResourceHandler::loadImage( 
		key + "_off", fs::canonical( offBtnFilePath ).string( ), premultipliedAlpha
	);

	imageOn_ = ResourceHandler::loadImage(
		key + "_on", fs::canonical( onBtnFilePath ).string( ), premultipliedAlpha
	);
}

void Button::update( ) {
	auto bLbtnTap = InputHandler::getKeyState( Key::MouseLbtn ) == KeyState::Tap;
	auto bLbtnAway = InputHandler::getKeyState( Key::MouseLbtn ) == KeyState::Away;
	
	if ( mouseOnCheck( ) ) {
		if ( bLbtnTap ) {
			bLbtnDown_ = true;
		}
		else if ( bLbtnAway ) {
			if ( bLbtnDown_ ) {
				mouseLbtnClicked( );
			}
			bLbtnDown_ = false;
		}
	}
	else {
		if ( bLbtnAway ) {
			bLbtnDown_ = false;
		}
	}

	componentUpdate( );
}

void Button::update( bool selected ) {
    // 서버 선택 표시만 반영하고, 마우스 입력은 프레임 update()에서 한 번 처리한다.
	resetImageState( );
	if ( selected ) {
		imageOff_ = imageOn_;
	}

	componentUpdate( );
}

void Button::render( HDC hdc ) {
	auto relativePos = pos_ - Camera::getPos( );

	if ( bMouseOn_ ) {
		imageOn_->draw( hdc, relativePos );
	}
	else {
		imageOff_->draw( hdc, relativePos );
	}

	componentRender( hdc );
}

bool Button::mouseOnCheck( ) {
	auto mousePos = InputHandler::getMousePos( );

	auto objPos = getPos( );
	auto objScale = PointFloat( 
		static_cast<float>( imageOff_->width( ) ),
		static_cast<float>( imageOff_->height( ) )
	);

	if ( mousePos.x >= objPos.x - objScale.x / 2.f && mousePos.x <= objPos.x + objScale.x / 2.f
		&& mousePos.y >= objPos.y - objScale.y / 2.f && mousePos.y <= objPos.y + objScale.y / 2.f ) {
		bMouseOn_ = true;
		return true;
	}
	else {
		bMouseOn_ = false;
		return false;
	}
}

void Button::mouseLbtnClicked( ) {
	auto name = getName( );

	if ( name == "arcade_btn" ) {
		EventHandler::addEvent(
			Event{
				.type = EventType::ChangeScene,
				.wParam = static_cast<DWORD_PTR>( SceneType::SelectCharacter )
			}
		);
	}
	else if ( name == "left_up_button" ) {
		EventHandler::addEvent(
			Event{
				.type = EventType::UpdateIndex,
				.wParam = static_cast<DWORD_PTR> ( 1 ),
				.lParam = reinterpret_cast<DWORD_PTR>( SceneHandler::getCurrScene( )->getObjectByName( "image_storage_left" ) )
			}
		);
	}
	else if ( name == "right_up_button" ) {
		EventHandler::addEvent(
			Event{
				.type = EventType::UpdateIndex,
				.wParam = static_cast<DWORD_PTR> ( 1 ),
				.lParam = reinterpret_cast<DWORD_PTR>( SceneHandler::getCurrScene( )->getObjectByName( "image_storage_right" ) )
			}
		);
	}
	else if ( name == "left_down_button" ) {
		EventHandler::addEvent(
			Event{
				.type = EventType::UpdateIndex,
				.wParam = static_cast<DWORD_PTR> ( -1 ),
				.lParam = reinterpret_cast<DWORD_PTR>( SceneHandler::getCurrScene( )->getObjectByName( "image_storage_left" ) )
			}
		);
	}
	else if ( name == "right_down_button" ) {
		EventHandler::addEvent(
			Event{
				.type = EventType::UpdateIndex,
				.wParam = static_cast<DWORD_PTR> ( -1 ),
				.lParam = reinterpret_cast<DWORD_PTR>( SceneHandler::getCurrScene( )->getObjectByName( "image_storage_right" ) )
			}
		);
	}
	else if ( name == "character_back_button" ) {
		Game::sendSelection( false );
		EventHandler::addEvent(
			Event{
				.type = EventType::ChangeScene,
				.wParam = static_cast<DWORD_PTR>( SceneType::Title )
			}
		);
	}
	else if ( name == "character_next_button" ) {
		Game::toggleReady( );
	}
	else if ( name == "mini_stage1" ) {
		imageOff_ = imageOn_;
		stageNum = StageNumber::Stage1;

		auto btnGroup = SceneHandler::getCurrScene( )->getGroup( ObjectType::Button );
		for ( auto btn : btnGroup ) {
			if ( btn->getName( ) == "mini_stage2" || btn->getName( ) == "mini_stage3"
				|| btn->getName( ) == "mini_stage4" 
				) {
				static_cast<Button*>( btn )->resetImageState( );
			}
		}
	}
	else if ( name == "mini_stage2" ) {
		imageOff_ = imageOn_;
		stageNum = StageNumber::Stage2;

		auto btnGroup = SceneHandler::getCurrScene( )->getGroup( ObjectType::Button );
		for ( auto btn : btnGroup ) {
			if ( btn->getName( ) == "mini_stage1" || btn->getName( ) == "mini_stage3"
				|| btn->getName( ) == "mini_stage4"
				) {
				static_cast<Button*>( btn )->resetImageState( );
			}
		}
	}
	else if ( name == "mini_stage3" ) {
		imageOff_ = imageOn_;
		stageNum = StageNumber::Stage3;

		auto btnGroup = SceneHandler::getCurrScene( )->getGroup( ObjectType::Button );
		for ( auto btn : btnGroup ) {
			if ( btn->getName( ) == "mini_stage1" || btn->getName( ) == "mini_stage2"
				|| btn->getName( ) == "mini_stage4"
				) {
				static_cast<Button*>( btn )->resetImageState( );
			}
		}
	}
	else if ( name == "mini_stage4" ) {
		imageOff_ = imageOn_;
		stageNum = StageNumber::Stage4;

		auto btnGroup = SceneHandler::getCurrScene( )->getGroup( ObjectType::Button );
		for ( auto btn : btnGroup ) {
			if ( btn->getName( ) == "mini_stage1" || btn->getName( ) == "mini_stage2"
				|| btn->getName( ) == "mini_stage3"
				) {
				static_cast<Button*>( btn )->resetImageState( );
			}
		}
	}
	else if ( name == "stage_back_button" ) {
		stageNum = StageNumber::None;

		EventHandler::addEvent(
			Event{
				.type = EventType::ChangeScene,
				.wParam = static_cast<DWORD_PTR>( SceneType::SelectCharacter )
			}
		);
	}
	else if ( name == "stage_play_button" ) {
		if ( stageNum == StageNumber::None ) {
			return;
		}

		EventHandler::addEvent(
			Event{
				.type = EventType::ChangeScene,
				.wParam = static_cast<DWORD_PTR>( SceneType::InGame )
			}
		);
	}

	//std::cout << "stageNum: " << static_cast<int>( stageNum ) << std::endl;
}

void Button::resetImageState( ) {
	imageOn_ = ResourceHandler::findImage( getName( ) + "_on" );
	imageOff_ = ResourceHandler::findImage( getName( ) + "_off" );
}
