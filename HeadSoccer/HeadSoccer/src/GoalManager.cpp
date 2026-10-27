#include "../include/GoalManager.hpp"
#include "../include/ResourceHandler.hpp"
#include "../include/SceneHandler.hpp"
#include "../include/Timer.hpp"
#include "../include/Camera.hpp"
#include "../include/Player.hpp"
#include "../include/Ball.hpp"
#include "../include/TextLabel.hpp"

#pragma comment( lib, "msimg32" )

extern int p1GoalScore;
extern int p2GoalScore;

namespace {
	constexpr float goalScreenTime = 3.f;

	// the ball is in once its center is past the goal line and under the crossbar
	constexpr float leftGoalLine = 50.f;
	constexpr float rightGoalLine = 1230.f;
	constexpr float crossbarBottom = 420.f;

	// same as the spawn positions in InGameScene::entry
	const PointFloat ballStartPos( 640.f, 360.f );
	const PointFloat p1StartPos( 300.f, 500.f );
	const PointFloat p2StartPos( 980.f, 500.f );
}

GoalManager::GoalManager( )
	: Object( "goal_manager", PointFloat( ), PointFloat( ) ),
	digits_( ), lastScorer_{ 0 }, celebrateT_{ 0.f }, celebrating_{ false } {
	digits_.reserve( 10 );
	for ( auto i = 0; i < 10; ++i ) {
		auto fileName = "goal_score_" + std::to_string( i ) + ".png";
		auto digitImage = ResourceHandler::loadImage( fileName, "assets/" + fileName, true );

		assert( digitImage != nullptr && "Failed to load goal score image" );

		digits_.emplace_back( digitImage );
	}

	bigFont_ = CreateFontW( 160, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Arial Black" );
	smallFont_ = CreateFontW( 48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Arial Black" );

	// 1x1 black pixel, stretched with AlphaBlend to dim a band behind the text
	DWORD black = 0;
	bandBmp_ = CreateBitmap( 1, 1, 1, 32, &black );
	bandDC_ = CreateCompatibleDC( nullptr );
	oldBandBmp_ = (HBITMAP)SelectObject( bandDC_, bandBmp_ );
}

GoalManager::~GoalManager( ) {
	SelectObject( bandDC_, oldBandBmp_ );
	DeleteDC( bandDC_ );
	DeleteObject( bandBmp_ );
	DeleteObject( bigFont_ );
	DeleteObject( smallFont_ );
}

void GoalManager::update( ) {
	// play goes on during the goal screen, but no new goal is counted until kickoff
	if ( celebrating_ ) {
		celebrateT_ += Timer::fdt( );
		if ( celebrateT_ >= goalScreenTime ) {
			kickoff( );
		}
		return;
	}

	for ( Object* ball : SceneHandler::getCurrScene( )->getGroup( ObjectType::Ball ) ) {
		auto pos = ball->getPos( );
		if ( pos.y < crossbarBottom ) {
			continue;
		}

		// player2 attacks the left goal, player1 the right one
		if ( pos.x <= leftGoalLine ) {
			scoreGoal( 2 );
			return;
		}
		if ( pos.x >= rightGoalLine ) {
			scoreGoal( 1 );
			return;
		}
	}
}

void GoalManager::scoreGoal( int scorer ) {
	if ( scorer == 1 ) {
		++p1GoalScore;
	}
	else {
		++p2GoalScore;
	}

	lastScorer_ = scorer;
	celebrateT_ = 0.f;
	celebrating_ = true;
}

void GoalManager::kickoff( ) {
	auto currScene = SceneHandler::getCurrScene( );

	for ( Object* obj : currScene->getGroup( ObjectType::Ball ) ) {
		static_cast<Ball*>( obj )->reset( ballStartPos );
	}

	for ( Object* obj : currScene->getGroup( ObjectType::Player ) ) {
		auto player = static_cast<Player*>( obj );
		player->reset( player->getName( ) == "player1" ? p1StartPos : p2StartPos );
	}

	celebrating_ = false;
}

void GoalManager::render( HDC hdc ) {
	renderScore( hdc, p1GoalScore, PointFloat( 460.f, 130.f ) );
	renderScore( hdc, p2GoalScore, PointFloat( 820.f, 130.f ) );

	if ( celebrating_ ) {
		renderGoalScreen( hdc );
	}
}

void GoalManager::renderScore( HDC hdc, int score, PointFloat pos ) {
	auto relativePos = pos - Camera::getPos( );

	if ( score < 10 ) {
		digits_[ score ]->draw( hdc, relativePos );
		return;
	}

	// two digits side by side, centered on pos
	auto half = digits_[ 0 ]->width( ) / 2.f;
	digits_[ score / 10 % 10 ]->draw( hdc, PointFloat( relativePos.x - half, relativePos.y ) );
	digits_[ score % 10 ]->draw( hdc, PointFloat( relativePos.x + half, relativePos.y ) );
}

void GoalManager::renderGoalScreen( HDC hdc ) {
	auto blend = BLENDFUNCTION{ AC_SRC_OVER, 0, 150, 0 };
	AlphaBlend( hdc, 0, 220, 1280, 240, bandDC_, 0, 0, 1, 1, blend );

	auto oldFont = (HFONT)SelectObject( hdc, bigFont_ );
	auto oldMode = SetBkMode( hdc, TRANSPARENT );
	auto oldAlign = SetTextAlign( hdc, TA_CENTER | TA_TOP );

	TextLabel::drawOutlined( hdc, L"GOAL!", 640, 235, RGB( 255, 220, 0 ) );

	SelectObject( hdc, smallFont_ );
	TextLabel::drawOutlined( hdc, lastScorer_ == 1 ? L"PLAYER 1" : L"PLAYER 2", 640, 400, RGB( 255, 255, 255 ) );

	SetTextAlign( hdc, oldAlign );
	SetBkMode( hdc, oldMode );
	SelectObject( hdc, oldFont );
}
