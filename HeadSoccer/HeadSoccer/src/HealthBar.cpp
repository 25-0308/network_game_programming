#include "../include/HealthBar.hpp"
#include "../include/Player.hpp"
#include "../include/TextLabel.hpp"
#include "../include/Camera.hpp"

namespace {
	constexpr int barWidth = 300;
	constexpr int barHeight = 24;
	constexpr int barMargin = 40;
	constexpr int barTop = 30;
	constexpr int screenWidth = 1280;
}

HealthBar::HealthBar( const Player* player, bool rightSide )
	: player_{ player }, rightSide_{ rightSide } {
	font_ = CreateFontW( 36, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Arial Black" );

	bgBrush_ = CreateSolidBrush( RGB( 60, 60, 60 ) );
	highBrush_ = CreateSolidBrush( RGB( 60, 200, 60 ) );
	midBrush_ = CreateSolidBrush( RGB( 240, 200, 0 ) );
	lowBrush_ = CreateSolidBrush( RGB( 220, 40, 40 ) );
}

HealthBar::~HealthBar( ) {
	DeleteObject( font_ );
	DeleteObject( bgBrush_ );
	DeleteObject( highBrush_ );
	DeleteObject( midBrush_ );
	DeleteObject( lowBrush_ );
}

void HealthBar::render( HDC hdc ) {
	auto left = rightSide_ ? screenWidth - barMargin - barWidth : barMargin;
	auto bar = RECT{ left, barTop, left + barWidth, barTop + barHeight };
	FillRect( hdc, &bar, bgBrush_ );

	// the bar drains toward the outer edge of the screen
	auto ratio = player_->getHp( ) / Player::maxHp;
	auto fillWidth = static_cast<LONG>( barWidth * ratio );
	auto fill = bar;
	if ( rightSide_ ) {
		fill.left = bar.right - fillWidth;
	}
	else {
		fill.right = bar.left + fillWidth;
	}

	auto fillBrush = ratio > 0.5f ? highBrush_ : ( ratio > 0.25f ? midBrush_ : lowBrush_ );
	FillRect( hdc, &fill, fillBrush );
	FrameRect( hdc, &bar, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

	if ( !player_->isStunned( ) ) {
		return;
	}

	auto oldFont = (HFONT)SelectObject( hdc, font_ );
	auto oldMode = SetBkMode( hdc, TRANSPARENT );
	auto oldAlign = SetTextAlign( hdc, TA_CENTER | TA_TOP );

	auto headPos = player_->getPos( ) - Camera::getPos( );
	TextLabel::drawOutlined( hdc, L"STUN",
		static_cast<int>( headPos.x ), static_cast<int>( headPos.y - 100.f ), RGB( 255, 220, 0 ) );

	SetTextAlign( hdc, oldAlign );
	SetBkMode( hdc, oldMode );
	SelectObject( hdc, oldFont );
}
