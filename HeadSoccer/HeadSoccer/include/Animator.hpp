#ifndef __animator_hpp
#define __animator_hpp

#include "common.hpp"
#include "Animation.hpp"

class Animator {
public:
	Animator( ) : anims_( ), currAnim_{ nullptr } {}

	// need to implement a copy constructor and a move constructor

	Animator& operator=( const Animator& ) = delete;
	Animator& operator=( Animator&& ) = delete;

	~Animator( ) {
		for ( auto [null, anim] : anims_ ) {
			delete anim;
		}
		anims_.clear( );
	}

	void update( ) {
		currAnim_->update( );
	}

	void render( HDC hdc, PointFloat objPos ) {
		currAnim_->render( hdc, objPos );
	}

	void createAnimation( const std::string& name, Image* image,
		PointFloat lt, PointFloat sliceSize, PointFloat step,
		std::uint8_t frameCnt, float duration, PointFloat offset = { 0.0f, 0.0f } );
	Animation* find( const std::string& name );
	void play( const std::string& name );

private:
	std::map<std::string, Animation*> anims_;
	Animation* currAnim_;
};

#endif	// __animator_hpp