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
#include "../include/CharacterInfo.hpp"

#include <cstdio>
#include <optional>

extern CharacterInfo player1;
extern CharacterInfo player2;

namespace {
	// "korea_left.png" 같은 캐릭터 이미지 이름 -> Protocol::Character
	std::optional<Protocol::Character> characterFromImage( const std::string& imageName ) {
		using Protocol::Character;
		constexpr std::pair<const char*, Character> names[ ] = {
			{ "america_", Character::America },
			{ "argentina_", Character::Argentina },
			{ "france_", Character::France },
			{ "korea_", Character::Korea },
		};
		for ( const auto& [ prefix, character ] : names ) {
			if ( imageName.starts_with( prefix ) ) {
				return character;
			}
		}
		return std::nullopt;
	}
}

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
	sendSelection( );

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

void Game::sendSelection( ) {
	auto playerNum = ClientNetwork::getPlayerNum( );
	if ( playerNum == 0 ) {
		return;
	}

	auto character = sentCharacter_;
	bool ready = false;

	const auto sceneType = SceneHandler::getCurrSceneType( );
	switch ( sceneType ) {
	case SceneType::SelectCharacter: {
		// 1P는 왼쪽, 2P는 오른쪽 캐릭터를 고름, 고르는 중에도 보내서 상대에게 보이게 함
		// ImageStorage의 인덱스는 이름순이라 Protocol::Character 순서와 같음
		auto storage = SceneHandler::getCurrScene( )->getObjectByName(
			playerNum == 1 ? "image_storage_left" : "image_storage_right" );
		if ( storage != nullptr ) {
			character = static_cast<Protocol::Character>( static_cast<ImageStorage*>( storage )->getIndex( ) );
		}
		break;
	}
	case SceneType::SelectStage:
	case SceneType::InGame: {
		// 다음 버튼에서 확정한 캐릭터 (InGameScene이 Player를 만들 때 쓰는 정보와 같음)
		const auto& info = playerNum == 1 ? player1 : player2;
		if ( auto confirmed = characterFromImage( info.getImage( ) ) ) {
			character = *confirmed;
		}
		ready = sceneType == SceneType::InGame;
		break;
	}
	default:
		// 타이틀, 결과 화면은 준비 해제만 함
		break;
	}

	// 바뀔 때만 보냄
	if ( character == sentCharacter_ && ready == sentReady_ ) {
		return;
	}

	auto select = Protocol::C2S_SelectCharacter{ };
	select.character = character;
	select.confirmed = ready ? 1 : 0;
	if ( !ClientNetwork::sendPacket( &select, sizeof( select ) ) ) {
		// 끊긴 연결은 다음 poll에서 처리
		std::printf( "C2S_SelectCharacter 전송 실패\n" );
		return;
	}
	sentCharacter_ = character;
	sentReady_ = ready;
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
	case Protocol::PacketType::S2C_CharacterSelected: {
		if ( packet.size( ) == sizeof( Protocol::S2C_CharacterSelected ) ) {
			auto selected = reinterpret_cast<const Protocol::S2C_CharacterSelected*>( packet.data( ) );
			// 내 선택은 서버 값으로 맞춤: 경기 종료나 상대 퇴장으로 서버가 준비를 풀면
			// 아직 경기 화면에 있을 때 다음 sendSelection에서 다시 준비를 보냄
			// (상대 선택 표시는 캐릭터 선택 화면 연동에서 처리)
			if ( selected->playerNum == ClientNetwork::getPlayerNum( ) ) {
				sentCharacter_ = selected->character;
				sentReady_ = selected->confirmed != 0;
			}
		}
		break;
	}
	case Protocol::PacketType::S2C_GameStart: {
		if ( packet.size( ) == sizeof( Protocol::S2C_GameStart ) ) {
			// 서버는 경기 시작 때 키 상태를 0으로 비우고, 경기 전에 보낸 C2S_Input은 버림
			// 키를 누른 채 시작하면 바뀌지 않아서 다시 보내지 않으므로 서버와 같이 0으로 맞춰 다음 프레임에 다시 보냄
			sentKeys_ = 0;
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

// 서버도 접속 시 America, 준비 안 함으로 시작
Protocol::Character Game::sentCharacter_ = Protocol::Character::America;
bool Game::sentReady_ = false;
