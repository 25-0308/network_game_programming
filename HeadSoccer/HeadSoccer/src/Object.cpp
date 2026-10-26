#include "../include/Object.hpp"

Object::Object( )
	: name_( ), pos_( ),
	scale_( ), alive_{ true },
	animator_{ nullptr }, collider_{ nullptr } {}

Object::Object( const std::string& name, const PointFloat& pos, const PointFloat& scale )
	: name_( name ), pos_( pos ),
	scale_( scale ), alive_{ true },
	animator_{ nullptr }, collider_{ nullptr } {}

Object::Object( const Object& other )
	: name_( other.name_ ), pos_( other.pos_ ),
	scale_( other.scale_ ),	alive_{ true },
	animator_{ nullptr }, collider_{ nullptr }
{
	if ( other.animator_ ) {
		createAnimator( );
		// *animator_ = *other.animator_;	복사 할당 연산자도 구현해야 할 듯? Animator, Animation 둘 다
	}
	if ( other.collider_ ) {
		createCollider( );
		collider_->setOffset( other.collider_->getOffset( ) );
		collider_->setScale( other.collider_->getScale( ) );
	}
}

Object::Object( Object&& other ) noexcept
	: name_( std::move( other.name_ ) ),
	pos_( std::move( other.pos_ ) ),
	scale_( std::move( other.scale_ ) ),
	alive_{ std::exchange( other.alive_, false ) },
	animator_{ std::exchange( other.animator_, nullptr ) },
	collider_{ std::exchange( other.collider_, nullptr ) } {}
