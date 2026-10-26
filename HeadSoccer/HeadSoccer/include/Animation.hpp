#ifndef __animation_hpp
#define __animation_hpp

#include "common.hpp"
#include "Image.hpp"

struct FrameInfo {
	PointFloat lt;	// left-top
	PointFloat sliceSize;
	PointFloat offset;
	float duration;
};

class Animation {
public:
	Animation( std::string_view name, Image* image,
		PointFloat lt, PointFloat sliceSize, PointFloat step,
		std::uint8_t frameCnt, float duration, PointFloat offset = { 0.0f, 0.0f } )
		: name_( name ), image_( image ), frames_( ), currFrame_{ 0 }, accT_{ 0.f }
	{
		assert( image_ != nullptr );

		frames_.reserve( frameCnt );
		for ( auto i = 0; i < frameCnt; ++i ) {
			frames_.emplace_back(
				lt + ( step * i ),
				sliceSize,
				offset,
				duration
			);
		}
	}

	// need to implement a copy constructor and a move constructor

	Animation& operator=( const Animation& ) = delete;
	Animation& operator=( Animation&& ) = delete;

	~Animation( ) {}

	void update( );
	void render( HDC hdc, PointFloat objPos );

	const std::string& name( ) const {
		return name_;
	}

private:
	std::string name_;
	Image* image_;

	std::vector<FrameInfo> frames_;
	std::uint8_t currFrame_;
	
	float accT_;
};

#endif	// __animation_hpp