#ifndef __camera_hpp
#define __camera_hpp

#include "PointFloat.hpp"

class Camera {
public:
	static void setPos( PointFloat pos ) {
		pos_ = pos;
	}

	static PointFloat getPos( ) {
		return pos_;
	}

private:
	static PointFloat pos_;
};

#endif	// __camera_hpp