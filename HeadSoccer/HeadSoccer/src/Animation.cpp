#include "../include/Animation.hpp"
#include "../include/Timer.hpp"
#include "../include/Camera.hpp"

void Animation::update( ) {
	accT_ += Timer::fdt( );
	while ( accT_ >= frames_[ currFrame_ ].duration ) {
		accT_ -= frames_[ currFrame_++ ].duration;
		currFrame_ %= frames_.size( );
	}
}

void Animation::render( HDC hdc, PointFloat objPos ) {
	auto relativePos = objPos - Camera::getPos( );

	auto sliceSize = frames_[ currFrame_ ].sliceSize;
	auto offset = frames_[ currFrame_ ].offset;
	auto lt = frames_[ currFrame_ ].lt;

	image_->draw( hdc,
		static_cast<int>( relativePos.x - sliceSize.x / 2.f + offset.x ),
		static_cast<int>( relativePos.y - sliceSize.y / 2.f + offset.y ),
		static_cast<int>( sliceSize.x ),
		static_cast<int>( sliceSize.y ),
		static_cast<int>( lt.x ),
		static_cast<int>( lt.y ),
		static_cast<int>( sliceSize.x ),
		static_cast<int>( sliceSize.y ) 
	);

	//auto imageDc = image_->getDC( );

	//// ÁÂ¿ì ¹ÝÀü
	//StretchBlt( 
	//	hdc,
	//	static_cast<int>( relativePos.x - sliceSize.x / 2.f + offset.x + sliceSize.x ),
	//	static_cast<int>( relativePos.y - sliceSize.y / 2.f + offset.y ),
	//	-sliceSize.x, static_cast<int>( sliceSize.y ),
	//	imageDc,
	//	static_cast<int>( lt.x ),
	//	static_cast<int>( lt.y ),
	//	static_cast<int>( sliceSize.x ),
	//	static_cast<int>( sliceSize.y ),
	//	SRCCOPY 
	//);

	//image_->releaseDC( );
}
