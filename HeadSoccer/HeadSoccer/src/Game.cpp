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
#include "../include/ImageStorage.hpp"
#include "../include/Button.hpp"
#include "../include/TextLabel.hpp"

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

void Game::sendSelection( bool confirmed ) {
	auto num = ClientNetwork::getPlayerNum( );
	if ( num < 1 || num > Protocol::maxPlayers
		|| SceneHandler::getCurrSceneType( ) != SceneType::SelectCharacter ) {
		return;
	}

	auto name = num == 1 ? "image_storage_left" : "image_storage_right";
	auto storage = static_cast<ImageStorage*>( SceneHandler::getCurrScene( )->getObjectByName( name ) );
	if ( storage == nullptr ) {
		return;
	}

	auto select = Protocol::C2S_SelectCharacter{ };
	select.character = static_cast<Protocol::Character>( storage->getIndex( ) );
	select.confirmed = confirmed ? 1 : 0;
	if ( !ClientNetwork::sendPacket( &select, sizeof( select ) ) ) {
		std::printf( "C2S_SelectCharacter 전송 실패\n" );
	}
}

bool Game::canBack( ) {
	auto num = ClientNetwork::getPlayerNum( );
	return num < 1 || num > Protocol::maxPlayers || characterSelections_[ num - 1 ].confirmed == 0;
}

void Game::toggleReady( ) {
	auto num = ClientNetwork::getPlayerNum( );
	if ( num < 1 || num > Protocol::maxPlayers ) {
		return;
	}
	sendSelection( characterSelections_[ num - 1 ].confirmed == 0 );
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
	case Protocol::PacketType::S2C_ChangeScene: {
		if ( packet.size( ) == sizeof( Protocol::S2C_ChangeScene ) ) {
			EventHandler::addEvent(
				Event{
					.type = EventType::ChangeScene,
					.wParam = static_cast<DWORD_PTR>( SceneType::SelectCharacter )
				}
			);
			// 뒤따르는 선택 상태 패킷보다 먼저 화면 전환을 완료한다.
			EventHandler::update( );
		}
		break;
	}
	case Protocol::PacketType::S2C_CharacterSelected: {
		if ( packet.size( ) == sizeof( Protocol::S2C_CharacterSelected ) ) {
			applySelection( *reinterpret_cast<const Protocol::S2C_CharacterSelected*>( packet.data( ) ) );
		}
		break;
	}
	case Protocol::PacketType::S2C_Frame: {
		if ( packet.size( ) == sizeof( Protocol::S2C_Frame ) ) {
			auto frame = reinterpret_cast<const Protocol::S2C_Frame*>( packet.data( ) );
			Timer::setFrame( *frame );
			if ( SceneHandler::getCurrSceneType( ) == SceneType::SelectCharacter ) {
				auto countdown = static_cast<TextLabel*>( SceneHandler::getCurrScene( )->getObjectByName( "start_countdown" ) );
				if ( countdown ) {
					countdown->setText( frame->countdown >= 1 && frame->countdown <= 3
						? std::to_wstring( frame->countdown ) : L"" );
				}
			}
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

void Game::applySelection( const Protocol::S2C_CharacterSelected& selected ) {
	if ( selected.playerNum < 1 || selected.playerNum > Protocol::maxPlayers
		|| static_cast<std::uint8_t>( selected.character ) > static_cast<std::uint8_t>( Protocol::Character::Korea )
		|| selected.confirmed > 1 ) {
		return;
	}

	characterSelections_[ selected.playerNum - 1 ] = selected;
	if ( SceneHandler::getCurrSceneType( ) != SceneType::SelectCharacter ) {
		return;
	}

	auto scene = SceneHandler::getCurrScene( );
	auto name = selected.playerNum == 1 ? "image_storage_left" : "image_storage_right";
	auto storage = static_cast<ImageStorage*>( scene->getObjectByName( name ) );
	if ( storage == nullptr ) {
		return;
	}
	auto index = static_cast<std::int32_t>( selected.character );
	const bool ownSelection = selected.playerNum == ClientNetwork::getPlayerNum( );
	storage->update( index );

	auto labelName = selected.playerNum == 1 ? "player1_selection_state" : "player2_selection_state";
	if ( auto label = static_cast<TextLabel*>( scene->getObjectByName( labelName ) ) ) {
		auto text = std::wstring( selected.playerNum == 1 ? L"1P " : L"2P " );
		if ( ownSelection ) {
			text += L"(YOU) ";
		}
		text += selected.confirmed ? L"READY" : L"SELECTING";
		label->setText( text );
	}
	if ( ownSelection ) {
		if ( auto button = static_cast<Button*>( scene->getObjectByName( "character_next_button" ) ) ) {
			button->update( selected.confirmed != 0 );
		}
	}
}

void Game::restoreSelection( ) {
	if ( SceneHandler::getCurrSceneType( ) != SceneType::SelectCharacter ) {
		return;
	}
	for ( const auto& selected : characterSelections_ ) {
		applySelection( selected );
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
std::array<Protocol::S2C_CharacterSelected, Protocol::maxPlayers> Game::characterSelections_{
	Protocol::S2C_CharacterSelected{ .playerNum = 1 },
	Protocol::S2C_CharacterSelected{ .playerNum = 2 }
};
