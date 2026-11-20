#include "../include/Game.hpp"
#include "../include/ClientNetwork.hpp"
#include "../include/Timer.hpp"
#include "../include/InputHandler.hpp"
#include "../include/SceneHandler.hpp"
#include "../include/EventHandler.hpp"
#include "../include/ResourceHandler.hpp"
#include "../include/CollisionHandler.hpp"
#include "../include/SoundHandler.hpp"
#include "../include/Player.hpp"

#include <cstdio>

bool Game::init( HWND hWnd, POINT rsl, const wchar_t* serverIp ) {
	hWnd_ = hWnd;

	// 시간은 서버가 관리하므로 접속이 안 되면 실행하지 않음
	if ( !ClientNetwork::connect( serverIp, Protocol::serverPort ) ) {
		MessageBoxW( hWnd_, L"서버에 접속할 수 없습니다.", L"Head Soccer", MB_OK | MB_ICONERROR );
		return false;
	}

	hDC_ = GetDC( hWnd_ );
	rsl_ = rsl;

	clientWidth_ = static_cast<std::uint16_t>( rsl_.x );
	clientHeight_ = static_cast<std::uint16_t>( rsl_.y );

	auto winRt = RECT{
		.left = 0,
		.top = 0,
		.right = rsl_.x,
		.bottom = rsl_.y
	};
	AdjustWindowRect( &winRt, WS_OVERLAPPEDWINDOW, false );
	SetWindowPos( hWnd_, nullptr, 100, 50,
		winRt.right - winRt.left, winRt.bottom - winRt.top, 0 );

	memDC_ = CreateCompatibleDC( hDC_ );
	memBmp_ = CreateCompatibleBitmap( hDC_, rsl_.x, rsl_.y );

	auto oldBmp = (HBITMAP)SelectObject( memDC_, memBmp_ );
	DeleteObject( oldBmp );

	InputHandler::init( );
	SoundHandler::init( );
	SceneHandler::init( );

	return true;
}

void Game::start( ) {
	if ( !ClientNetwork::isConnected( ) ) {
		return;
	}

	if ( !ClientNetwork::poll( ) ) {
		ClientNetwork::release( );
		MessageBoxW( hWnd_, L"서버와 연결이 끊겼습니다.", L"Head Soccer", MB_OK | MB_ICONERROR );
		PostQuitMessage( 0 );
		return;
	}

	// 처리 지점: 렌더 전에 받은 패킷을 순서대로 처리
	// (S2C_Frame마다 한 프레임 갱신, S2C_GameState는 Player에 서버 상태 적용)
	auto packet = std::vector<char>( );
	while ( ClientNetwork::popPacket( packet ) ) {
		handlePacket( packet );
	}

	// double buffering
	Rectangle( memDC_, -1, -1, rsl_.x + 1, rsl_.y + 1 );
	SceneHandler::render( memDC_ );
	BitBlt( hDC_, 0, 0, rsl_.x, rsl_.y, memDC_, 0, 0, SRCCOPY );

	Timer::render( );
}

void Game::release( ) {
	ReleaseDC( hWnd_, hDC_ );
	DeleteDC( memDC_ );
	DeleteObject( memBmp_ );

	SceneHandler::release( );
	SoundHandler::release( );
	ResourceHandler::release( );

	if ( ClientNetwork::isConnected( ) ) {
		ClientNetwork::release( );
	}
}

void Game::step( ) {
	InputHandler::update( );
	sendInput( );

	SceneHandler::update( );
	CollisionHandler::update( );

	EventHandler::update( );
}

void Game::sendInput( ) {
	// 자리를 배정받기 전에는 보낼 대상이 없음
	if ( ClientNetwork::getPlayerNum( ) == 0 ) {
		return;
	}

	// 1P, 2P 모두 자기 키는 A, D, W, S (상대 키는 읽지도 보내지도 않음)
	// 경기 중이 아니면 0으로 보내서, 누른 채 씬이 바뀌어도 서버에 키가 남지 않게 함
	auto keys = std::uint8_t{ 0 };
	if ( SceneHandler::getCurrSceneType( ) == SceneType::InGame ) {
		auto isDown = [ ]( Key key ) {
			auto state = InputHandler::getKeyState( key );
			return state == KeyState::Tap || state == KeyState::Hold;
		};

		if ( isDown( Key::A ) ) keys |= Protocol::KeyLeft;
		if ( isDown( Key::D ) ) keys |= Protocol::KeyRight;
		if ( isDown( Key::W ) ) keys |= Protocol::KeyJump;
		if ( isDown( Key::S ) ) keys |= Protocol::KeyKick;
	}

	// 바뀔 때만 보냄
	if ( keys == sentKeys_ ) {
		return;
	}

	auto input = Protocol::C2S_Input{ };
	input.keys = keys;
	if ( !ClientNetwork::sendPacket( &input, sizeof( input ) ) ) {
		// 끊긴 연결은 다음 poll에서 처리
		std::printf( "C2S_Input 전송 실패\n" );
		return;
	}
	sentKeys_ = keys;
}

void Game::handlePacket( const std::vector<char>& packet ) {
	auto header = reinterpret_cast<const Protocol::PacketHeader*>( packet.data( ) );

	switch ( header->type ) {
	case Protocol::PacketType::S2C_Login: {
		if ( packet.size( ) == sizeof( Protocol::S2C_Login ) ) {
			auto login = reinterpret_cast<const Protocol::S2C_Login*>( packet.data( ) );
			ClientNetwork::setPlayerNum( login->playerNum );
			std::printf( "플레이어 %d로 접속\n", login->playerNum );
		}
		break;
	}
	case Protocol::PacketType::S2C_Frame: {
		if ( packet.size( ) == sizeof( Protocol::S2C_Frame ) ) {
			Timer::setFrame( *reinterpret_cast<const Protocol::S2C_Frame*>( packet.data( ) ) );
			step( );
		}
		break;
	}
	case Protocol::PacketType::S2C_GameState: {
		if ( packet.size( ) == sizeof( Protocol::S2C_GameState ) ) {
			applyPlayerStates( *reinterpret_cast<const Protocol::S2C_GameState*>( packet.data( ) ) );
		}
		break;
	}
	default:
		// 나머지 패킷은 이후 일정에서 처리
		break;
	}
}

void Game::applyPlayerStates( const Protocol::S2C_GameState& state ) {
	// 경기 중이 아니면 Player 오브젝트가 없음
	if ( SceneHandler::getCurrSceneType( ) != SceneType::InGame ) {
		return;
	}

	// 원격 Player를 먼저 적용: 상대 발차기 이벤트가 내 Player를 맞히더라도
	// 바로 뒤에 내 Player에 서버 HP가 덮여서 이번 틱 결과와 같아짐
	auto& players = SceneHandler::getCurrScene( )->getGroup( ObjectType::Player );
	for ( bool remotePass : { true, false } ) {
		for ( Object* obj : players ) {
			auto player = static_cast<Player*>( obj );
			if ( player->isRemote( ) != remotePass ) {
				continue;
			}

			// players[0]은 1P, players[1]은 2P
			auto num = player->getPlayerNum( );
			if ( num < 1 || num > Protocol::maxPlayers ) {
				continue;
			}
			player->update( state.players[ num - 1 ] );
		}
	}
}

HWND Game::hWnd_;
HDC Game::hDC_;
POINT Game::rsl_;

std::uint16_t Game::clientWidth_;
std::uint16_t Game::clientHeight_;

HDC Game::memDC_;
HBITMAP Game::memBmp_;

std::uint8_t Game::sentKeys_ = 0;
