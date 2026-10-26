#include "../include/Game.hpp"
#include "../include/Timer.hpp"
#include "../include/InputHandler.hpp"
#include "../include/SceneHandler.hpp"
#include "../include/EventHandler.hpp"
#include "../include/ResourceHandler.hpp"
#include "../include/CollisionHandler.hpp"

void Game::init( HWND hWnd, POINT rsl ) {
	hWnd_ = hWnd;
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

	Timer::init( );
	InputHandler::init( );
	SceneHandler::init( );
}

void Game::start( ) {
	Timer::update( );
	InputHandler::update( );

	SceneHandler::update( );
	CollisionHandler::update( );

	// double buffering
	Rectangle( memDC_, -1, -1, rsl_.x + 1, rsl_.y + 1 );
	SceneHandler::render( memDC_ );
	BitBlt( hDC_, 0, 0, rsl_.x, rsl_.y, memDC_, 0, 0, SRCCOPY );

	Timer::render( );
	EventHandler::update( );
}

void Game::release( ) {
	ReleaseDC( hWnd_, hDC_ );
	DeleteDC( memDC_ );
	DeleteObject( memBmp_ );

	SceneHandler::release( );
	ResourceHandler::release( );
}

HWND Game::hWnd_;
HDC Game::hDC_;
POINT Game::rsl_;

std::uint16_t Game::clientWidth_;
std::uint16_t Game::clientHeight_;

HDC Game::memDC_;
HBITMAP Game::memBmp_;
