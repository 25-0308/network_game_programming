#include "../include/Game.hpp"
#include "../include/ClientNetwork.hpp"
#include "../include/Timer.hpp"
#include "../include/InputHandler.hpp"
#include "../include/SceneHandler.hpp"
#include "../include/EventHandler.hpp"
#include "../include/ResourceHandler.hpp"
#include "../include/SoundHandler.hpp"
#include "../include/Player.hpp"
#include "../include/Ball.hpp"
#include "../include/ImageStorage.hpp"
#include "../include/Button.hpp"
#include "../include/TextLabel.hpp"
#include "../include/CharacterInfo.hpp"
#include "../include/GoalManager.hpp"
#include "GameConstants.hpp"

#include <cstdio>

extern CharacterInfo player1;
extern CharacterInfo player2;
extern StageNumber stageNum;
extern int p1GoalScore;
extern int p2GoalScore;

bool Game::init( HWND hWnd, POINT rsl, const wchar_t* serverIp ) {
	hWnd_ = hWnd;

	// 시간은 서버가 관리하므로 접속이 안 되면 실행하지 않음
	if ( !ClientNetwork::connect( serverIp, Protocol::serverPort ) ) {
		MessageBoxW( hWnd_, L"서버에 접속할 수 없습니다.", L"Head Soccer", MB_OK | MB_ICONERROR );
		return false;
	}

	hDC_ = GetDC( hWnd_ );
	rsl_ = rsl;

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
	case Protocol::PacketType::S2C_GameStart: {
		if ( packet.size( ) == sizeof( Protocol::S2C_GameStart ) ) {
			startMatch( *reinterpret_cast<const Protocol::S2C_GameStart*>( packet.data( ) ) );
		}
		break;
	}
	case Protocol::PacketType::S2C_GameEnd: {
		if ( packet.size( ) == sizeof( Protocol::S2C_GameEnd ) ) {
			finishMatch( *reinterpret_cast<const Protocol::S2C_GameEnd*>( packet.data( ) ) );
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
	case Protocol::PacketType::S2C_Ball: {
		if ( packet.size( ) == sizeof( Protocol::S2C_Ball )
			&& SceneHandler::getCurrSceneType( ) == SceneType::InGame ) {
			auto state = reinterpret_cast<const Protocol::S2C_Ball*>( packet.data( ) );
			for ( Object* obj : SceneHandler::getCurrScene( )->getGroup( ObjectType::Ball ) ) {
				static_cast<Ball*>( obj )->update( *state );
			}
		}
		break;
	}
	case Protocol::PacketType::S2C_GameState: {
		if ( packet.size( ) == sizeof( Protocol::S2C_GameState ) ) {
			applyPlayerStates( *reinterpret_cast<const Protocol::S2C_GameState*>( packet.data( ) ) );
		}
		break;
	}
	case Protocol::PacketType::S2C_Goal: {
		if ( packet.size( ) == sizeof( Protocol::S2C_Goal ) ) {
			applyGoal( *reinterpret_cast<const Protocol::S2C_Goal*>( packet.data( ) ) );
		}
		break;
	}
	default:
		// 나머지 패킷은 이후 일정에서 처리
		break;
	}
}

void Game::startMatch( const Protocol::S2C_GameStart& start ) {
	if ( start.stage < 1 || start.stage > GameConst::stageCount
		|| static_cast<std::size_t>( start.p1Character ) >= GameConst::characterStats.size( )
		|| static_cast<std::size_t>( start.p2Character ) >= GameConst::characterStats.size( )
		|| SceneHandler::getCurrSceneType( ) == SceneType::InGame ) {
		return;
	}

	auto setCharacter = [&]( CharacterInfo& info, Protocol::Character character, const char* side ) {
		auto index = static_cast<std::size_t>( character );
		info.setImage( std::string( characterNames[ index ] ) + "_" + side + ".png" );
		info.setFlag( characterFlags[ index ] );
	};
	setCharacter( player1, start.p1Character, "left" );
	setCharacter( player2, start.p2Character, "right" );
	stageNum = static_cast<StageNumber>( start.stage );

	// 서버는 경기 시작 때 키 상태를 0으로 비우고, 경기 전에 보낸 C2S_Input은 버림
	// 키를 누른 채 시작하면 바뀌지 않아서 다시 보내지 않으므로 서버와 같이 0으로 맞춰 다음 프레임에 다시 보냄
	sentKeys_ = 0;

	EventHandler::addEvent(
		Event{
			.type = EventType::ChangeScene,
			.wParam = static_cast<DWORD_PTR>( SceneType::InGame )
		}
	);
	// 같은 수신 묶음의 첫 상태 패킷보다 먼저 경기 화면의 entry()를 끝낸다.
	EventHandler::update( );
}

void Game::finishMatch( const Protocol::S2C_GameEnd& end ) {
	auto type = SceneHandler::getCurrSceneType( );
	if ( type != SceneType::InGame && type != SceneType::Result ) {
		return;
	}
	const bool changed = p1GoalScore != end.scores[ 0 ] || p2GoalScore != end.scores[ 1 ];
	p1GoalScore = end.scores[ 0 ];
	p2GoalScore = end.scores[ 1 ];

	if ( type == SceneType::Result ) {
		if ( changed ) {
			auto scene = SceneHandler::getCurrScene( );
			scene->exit( );
			scene->entry( );
		}
		return;
	}

	EventHandler::addEvent(
		Event{
			.type = EventType::ChangeScene,
			.wParam = static_cast<DWORD_PTR>( SceneType::Result )
		}
	);
	// 최종 점수를 반영한 뒤 결과 화면의 entry()에서 승패를 표시한다.
	EventHandler::update( );
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

	// 피격 판정은 서버가 하므로 적용 순서와 관계없이 HP, 기절은 서버 값과 같음
	for ( Object* obj : SceneHandler::getCurrScene( )->getGroup( ObjectType::Player ) ) {
		auto player = static_cast<Player*>( obj );

		// players[0]은 1P, players[1]은 2P
		auto num = player->getPlayerNum( );
		if ( num < 1 || num > Protocol::maxPlayers ) {
			continue;
		}
		player->update( state.players[ num - 1 ] );
	}
}

void Game::applyGoal( const Protocol::S2C_Goal& goal ) {
	// 경기 중이 아니면 GoalManager가 없음
	if ( SceneHandler::getCurrSceneType( ) != SceneType::InGame ) {
		return;
	}

	auto goalManager = static_cast<GoalManager*>( SceneHandler::getCurrScene( )->getObjectByName( "goal_manager" ) );
	if ( goalManager == nullptr ) {
		return;
	}

	// 서버는 골마다 득점자 점수만 1 올리므로 지금 점수보다 커진 쪽이 득점자
	// 골 화면 문구(PLAYER 1, 2)와 골 효과음은 GoalManager::update가 처리
	int scorer = 0;
	if ( goal.scores[ 0 ] > p1GoalScore ) {
		scorer = 1;
	}
	else if ( goal.scores[ 1 ] > p2GoalScore ) {
		scorer = 2;
	}
	else {
		return;	// 이미 반영한 점수
	}

	goalManager->update( goal.scores[ 0 ], goal.scores[ 1 ], scorer, true );
}

HWND Game::hWnd_;
HDC Game::hDC_;
POINT Game::rsl_;

HDC Game::memDC_;
HBITMAP Game::memBmp_;

std::uint8_t Game::sentKeys_ = 0;
std::array<Protocol::S2C_CharacterSelected, Protocol::maxPlayers> Game::characterSelections_{
	Protocol::S2C_CharacterSelected{ .playerNum = 1 },
	Protocol::S2C_CharacterSelected{ .playerNum = 2 }
};
