#include "../include/Collider.hpp"
#include "../include/Camera.hpp"

std::uint16_t Collider::nextId_;

void Collider::render( HDC hdc ) const {
	auto oldPen = (HPEN)SelectObject( hdc, rPen_ );
	if ( collCnt_ > 0 ) {
		oldPen = (HPEN)SelectObject( hdc, gPen_ );
	}

	auto brush = (HBRUSH)GetStockObject( HOLLOW_BRUSH );
	auto oldBrush = (HBRUSH)SelectObject( hdc, brush );

	auto relativePos = finalPos_ - Camera::getPos( );

	Rectangle( hdc,
		static_cast<int>( relativePos.x - scale_.x / 2.f ),
		static_cast<int>( relativePos.y - scale_.y / 2.f ),
		static_cast<int>( relativePos.x + scale_.x / 2.f ),
		static_cast<int>( relativePos.y + scale_.y / 2.f )
	);

	SelectObject( hdc, oldPen );
	SelectObject( hdc, oldBrush );
}
