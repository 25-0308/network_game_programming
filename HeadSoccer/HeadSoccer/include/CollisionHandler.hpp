#ifndef __collision_handler_hpp
#define __collision_handler_hpp

#include "common.hpp"
#include "Collider.hpp"

union ColliderID {
	struct {
		std::uint32_t left;
		std::uint32_t right;
	} lr;
	std::uint64_t id;
};

class CollisionHandler {
public:
	static void update( );
	static void checkCollision( ObjectType lhs, ObjectType rhs );

	static void reset( ) {
		ZeroMemory( collTable_.data( ), sizeof( std::uint32_t ) * etoi( ObjectType::EOE ) );
	}

private:
	static std::array<std::uint32_t, etoi( ObjectType::EOE )> collTable_;
	static std::map<std::uint64_t, bool> collInfo_;

	static void collUpdate( ObjectType lhs, ObjectType rhs );
	static bool isCollided( Collider* lhs, Collider* rhs );
};

#endif	// __collision_handler_hpp