#ifndef __vec2_hpp
#define __vec2_hpp

#include <cmath>

// 서버용 2D 벡터 (클라이언트 PointFloat와 같은 역할)
// Windows.h, POINT 의존 없이 계산만 함
struct Vec2 {
	float x = 0.f;
	float y = 0.f;

	constexpr Vec2( ) = default;
	constexpr Vec2( float x, float y ) : x{ x }, y{ y } {}

	constexpr Vec2 operator+( Vec2 rhs ) const {
		return { x + rhs.x, y + rhs.y };
	}
	constexpr Vec2 operator-( Vec2 rhs ) const {
		return { x - rhs.x, y - rhs.y };
	}
	constexpr Vec2 operator-( ) const {
		return { -x, -y };
	}
	constexpr Vec2 operator*( float rhs ) const {
		return { x * rhs, y * rhs };
	}
	constexpr Vec2 operator/( float rhs ) const {
		return { x / rhs, y / rhs };
	}

	constexpr Vec2& operator+=( Vec2 rhs ) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	constexpr Vec2& operator-=( Vec2 rhs ) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	constexpr bool operator==( const Vec2& ) const = default;

	constexpr float dot( Vec2 rhs ) const {
		return x * rhs.x + y * rhs.y;
	}
	constexpr float lengthSq( ) const {
		return x * x + y * y;
	}
	float length( ) const {
		return std::hypot( x, y );
	}
};

#endif	// __vec2_hpp
