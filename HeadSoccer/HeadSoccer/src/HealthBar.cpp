#include "../include/HealthBar.hpp"
#include "../include/Player.hpp"
#include "../include/Camera.hpp"
#include "../include/Timer.hpp"

#include <numbers>

namespace {
	constexpr int barWidth = 300;
	constexpr int barHeight = 24;
	constexpr int barMargin = 40;
	constexpr int barTop = 30;
	constexpr int screenWidth = 1280;

	constexpr int starCount = 3;
	constexpr float starSpinSpeed = 4.f;	// radians per second
	constexpr float orbitRadiusX = 32.f;
	constexpr float orbitRadiusY = 8.f;
	constexpr float orbitHeight = 58.f;		// above the player's center, just over the head
	constexpr float starRadius = 9.f;
}

HealthBar::HealthBar( const Player* player, bool rightSide )
	: player_{ player }, rightSide_{ rightSide }, starAngle_{ 0.f } {
	bgBrush_ = CreateSolidBrush( RGB( 60, 60, 60 ) );
	highBrush_ = CreateSolidBrush( RGB( 60, 200, 60 ) );
	midBrush_ = CreateSolidBrush( RGB( 240, 200, 0 ) );
	lowBrush_ = CreateSolidBrush( RGB( 220, 40, 40 ) );
	starBrush_ = CreateSolidBrush( RGB( 255, 220, 0 ) );
	starPen_ = CreatePen( PS_SOLID, 2, RGB( 90, 60, 0 ) );
}

HealthBar::~HealthBar( ) {
	DeleteObject( bgBrush_ );
	DeleteObject( highBrush_ );
	DeleteObject( midBrush_ );
	DeleteObject( lowBrush_ );
	DeleteObject( starBrush_ );
	DeleteObject( starPen_ );
}

void HealthBar::update( ) {
	if ( player_->isStunned( ) ) {
		starAngle_ = std::fmod( starAngle_ + starSpinSpeed * Timer::fdt( ), 2.f * std::numbers::pi_v<float> );
	}
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

	if ( player_->isStunned( ) ) {
		renderStunStars( hdc );
	}
}

void HealthBar::renderStunStars( HDC hdc ) {
	constexpr auto pi = std::numbers::pi_v<float>;

	auto oldBrush = (HBRUSH)SelectObject( hdc, starBrush_ );
	auto oldPen = (HPEN)SelectObject( hdc, starPen_ );

	auto center = player_->getPos( ) - Camera::getPos( ) - PointFloat( 0.f, orbitHeight );

	for ( auto i = 0; i < starCount; ++i ) {
		float angle = starAngle_ + 2.f * pi * i / starCount;
		auto starPos = center + PointFloat( orbitRadiusX * std::cos( angle ), orbitRadiusY * std::sin( angle ) );

		// stars on the far side of the orbit (sin < 0) are drawn smaller for a bit of depth
		float radius = starRadius * ( 0.8f + 0.2f * std::sin( angle ) );

		// 5-pointed star, each star also spins on its own
		POINT pts[ 10 ];
		for ( auto p = 0; p < 10; ++p ) {
			float r = ( p % 2 == 0 ) ? radius : radius * 0.45f;
			float a = angle * 2.f - pi / 2.f + pi * p / 5.f;
			pts[ p ] = POINT{
				static_cast<LONG>( starPos.x + r * std::cos( a ) ),
				static_cast<LONG>( starPos.y + r * std::sin( a ) )
			};
		}
		Polygon( hdc, pts, 10 );
	}

	SelectObject( hdc, oldPen );
	SelectObject( hdc, oldBrush );
}
