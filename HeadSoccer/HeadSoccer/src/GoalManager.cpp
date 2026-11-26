#include "../include/GoalManager.hpp"
#include "../include/ResourceHandler.hpp"
#include "../include/SceneHandler.hpp"
#include "../include/Timer.hpp"
#include "../include/Camera.hpp"
#include "../include/Player.hpp"
#include "../include/TextLabel.hpp"
#include "../include/SoundHandler.hpp"
#include "GameConstants.hpp"

#pragma comment( lib, "msimg32" )

extern int p1GoalScore;
extern int p2GoalScore;

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
	// goals are detected by the server (Match::updateGoal), this only times the goal screen
	if ( !celebrating_ ) {
		return;
	}

	celebrateT_ += Timer::fdt( );
	if ( celebrateT_ >= GameConst::goalScreenTime ) {
		kickoff( );
	}
}

void GoalManager::update( int p1Score, int p2Score, int scorer, bool celebrating ) {
	if ( p1Score < 0 || p2Score < 0 || ( celebrating && scorer != 1 && scorer != 2 ) ) {
		return;
	}
	const bool newGoal = celebrating && ( !celebrating_ || lastScorer_ != scorer
		|| p1GoalScore != p1Score || p2GoalScore != p2Score );

	p1GoalScore = p1Score;
	p2GoalScore = p2Score;
	lastScorer_ = scorer;
	celebrating_ = celebrating;

	if ( newGoal ) {
		celebrateT_ = 0.f;
		SoundHandler::playEffect( SoundEffect::Goal );
	}
}

void GoalManager::kickoff( ) {
	auto currScene = SceneHandler::getCurrScene( );

	// the kickoff positions come from the server (Player::resetToKickoff) in S2C_GameState,
	// so keep the current position and only clear the local kick and dash sound state
	for ( Object* obj : currScene->getGroup( ObjectType::Player ) ) {
		auto player = static_cast<Player*>( obj );
		player->reset( player->getPos( ) );
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
