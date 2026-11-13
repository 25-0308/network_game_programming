#include "../include/Game.hpp"
#include "../include/ClientNetwork.hpp"
#include "../include/Timer.hpp"
#include "../include/InputHandler.hpp"
#include "../include/SceneHandler.hpp"
#include "../include/EventHandler.hpp"
#include "../include/ResourceHandler.hpp"
#include "../include/CollisionHandler.hpp"
#include "../include/SoundHandler.hpp"

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

	// 받은 패킷을 순서대로 처리 (S2C_Frame마다 한 프레임 갱신)
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

	SceneHandler::update( );
	CollisionHandler::update( );

	EventHandler::update( );
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
	default:
		// 나머지 패킷은 이후 일정에서 처리
		break;
	}
}

HWND Game::hWnd_;
HDC Game::hDC_;
POINT Game::rsl_;

std::uint16_t Game::clientWidth_;
std::uint16_t Game::clientHeight_;

HDC Game::memDC_;
HBITMAP Game::memBmp_;
