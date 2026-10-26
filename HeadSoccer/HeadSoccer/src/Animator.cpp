#include "../include/Animator.hpp"

void Animator::createAnimation( const std::string& name, Image* image,
	PointFloat lt, PointFloat sliceSize, PointFloat step,
	std::uint8_t frameCnt, float duration, PointFloat offset )
{
	auto anim = find( name );
	assert( anim == nullptr );

	anims_[ name ] = new Animation( name, image, lt, sliceSize, step, frameCnt, duration, offset );
}

Animation* Animator::find( const std::string& name ) {
	auto anim = anims_.find( name );

	if ( anim == anims_.end( ) ) {
		return nullptr;
	}

	assert( anim->second != nullptr );
	return anim->second;
}

void Animator::play( const std::string& name ) {
	currAnim_ = find( name );
	assert( currAnim_ != nullptr );
}
