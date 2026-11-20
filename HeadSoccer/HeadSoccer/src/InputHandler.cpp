#include "../include/common.hpp"
#include "../include/Game.hpp"
#include "../include/InputHandler.hpp"
#include "../include/SoundHandler.hpp"

#include <array>

auto arrVK = std::array<std::uint8_t, etoi( Key::EOE )>{ 
	VK_UP,
	VK_DOWN,
	VK_LEFT,
	VK_RIGHT,

	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
	'Z', 'X', 'C', 'V', 'B', 'N', 'M',

	VK_MENU,
	VK_CONTROL,
	VK_LSHIFT,
	VK_SPACE,
	VK_RETURN,
	VK_ESCAPE,

	VK_LBUTTON,
	VK_RBUTTON
};

void InputHandler::init( ) {
	inputInfo_.reserve( etoi( Key::EOE ) );
	for ( auto i = 0u; i < etoi( Key::EOE ); ++i ) {
		inputInfo_.emplace_back( KeyState::None, false );
	}
}

void InputHandler::update( ) {
	// GetAsyncKeyState는 다른 창에 친 키도 읽으므로, 포커스 창일 때만 키를 읽음
	// (한 PC에서 클라 두 개를 띄우면 양쪽이 같은 입력을 받게 됨)
	// 포커스를 잃으면 모든 키를 뗀 것으로 처리해서, 누르던 키는 Away를 거쳐 None이 됨
	const bool focused = GetForegroundWindow( ) == Game::getHwnd( );

	for ( auto i = 0u; i < etoi( Key::EOE ); ++i ) {
		if ( focused && ( GetAsyncKeyState( arrVK[ i ] ) & 0x8000 ) ) {
			if ( inputInfo_[ i ].isPressed ) {
				inputInfo_[ i ].state = KeyState::Hold;
			}
			else {
				inputInfo_[ i ].state = KeyState::Tap;
			}
			inputInfo_[ i ].isPressed = true;
		}
		else {
			if ( inputInfo_[ i ].isPressed ) {
				inputInfo_[ i ].state = KeyState::Away;
			}
			else {
				inputInfo_[ i ].state = KeyState::None;
			}
			inputInfo_[ i ].isPressed = false;
		}
	}

	POINT pt;
	GetCursorPos( &pt );
	ScreenToClient( Game::getHwnd( ), &pt );
	mousePos_ = static_cast<PointFloat>( pt );

	const auto resolution = Game::getRsl( );
	if ( getKeyState( Key::MouseLbtn ) == KeyState::Tap
		&& pt.x >= 0 && pt.y >= 0 && pt.x < resolution.x && pt.y < resolution.y ) {
		SoundHandler::playEffect( SoundEffect::Click );
	}
}

std::vector<InputInfo> InputHandler::inputInfo_;
PointFloat InputHandler::mousePos_;
