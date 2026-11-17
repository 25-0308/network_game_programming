#ifndef __aabb_hpp
#define __aabb_hpp

#include <algorithm>

#include "Vec2.hpp"

// 축 정렬 충돌 박스 (클라이언트 Collider를 대신함)
// 클라이언트 Collider와 같이 중심(finalPos) + 전체 크기(scale)로 표현
// GDI 펜, ID, 충돌 카운터 없이 값으로만 들고 다니고, 필요할 때 위치에서 바로 만들어 씀
struct AABB {
	Vec2 center;
	Vec2 size;

	constexpr Vec2 half( ) const {
		return size / 2.f;
	}
	constexpr float left( ) const {
		return center.x - size.x / 2.f;
	}
	constexpr float right( ) const {
		return center.x + size.x / 2.f;
	}
	constexpr float top( ) const {
		return center.y - size.y / 2.f;
	}
	constexpr float bottom( ) const {
		return center.y + size.y / 2.f;
	}

	// CollisionHandler::isCollided와 같은 판정 (변이 맞닿기만 하면 충돌 아님)
	constexpr bool overlaps( const AABB& other ) const {
		auto d = other.center - center;
		auto reach = ( size + other.size ) / 2.f;
		return absf( d.x ) < reach.x && absf( d.y ) < reach.y;
	}

	// 축마다 겹친 길이, 둘 다 0보다 크면 겹친 상태 (Player::separateFrom에서 사용)
	constexpr Vec2 overlapAmount( const AABB& other ) const {
		auto d = other.center - center;
		auto reach = ( size + other.size ) / 2.f;
		return { reach.x - absf( d.x ), reach.y - absf( d.y ) };
	}

	// 박스 안에서 p와 가장 가까운 점 (발차기 거리, 공과 박스 충돌에서 사용)
	constexpr Vec2 closestPoint( Vec2 p ) const {
		return { std::clamp( p.x, left( ), right( ) ), std::clamp( p.y, top( ), bottom( ) ) };
	}

	constexpr bool contains( Vec2 p ) const {
		return p.x >= left( ) && p.x <= right( ) && p.y >= top( ) && p.y <= bottom( );
	}

	// p에서 박스까지 거리 (안에 있으면 0)
	float distanceTo( Vec2 p ) const {
		return ( p - closestPoint( p ) ).length( );
	}

private:
	static constexpr float absf( float v ) {
		return v < 0.f ? -v : v;
	}
};

#endif	// __aabb_hpp
