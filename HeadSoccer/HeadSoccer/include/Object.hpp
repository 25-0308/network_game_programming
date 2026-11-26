#ifndef __object_hpp
#define __object_hpp

#include "common.hpp"
#include "Animator.hpp"

class Object {
public:
	Object( );
	Object( const std::string& name, const PointFloat& pos, const PointFloat& scale );

	Object( const Object& other );
	Object& operator=( const Object& other ) = delete;

	Object( Object&& other ) noexcept;
	Object& operator=( Object&& other ) = delete;

	virtual ~Object( ) = 0 {
		delete animator_;
	}

	virtual void update( ) {}
	virtual void render( HDC hdc ) {}

	virtual void componentUpdate( ) final {
		if ( animator_ ) {
			animator_->update( );
		}
	}

	virtual void componentRender( HDC hdc ) final {
		if ( animator_ ) {
			animator_->render( hdc, pos_ );
		}
	}

	void createAnimator( ) {
		animator_ = new Animator( );
	}

	void setName( const std::string& name ) {
		name_ = name;
	}
	void setPos( const PointFloat& pos ) {
		pos_ = pos;
	}
	void setScale( const PointFloat& scale ) {
		scale_ = scale;
	}

	const std::string& getName( ) const {
		return name_;
	}
	const PointFloat& getPos( ) const {
		return pos_;
	}
	const PointFloat& getScale( ) const {
		return scale_;
	}
	bool isAlive( ) const {
		return alive_;
	}

	Animator* getAnimator( ) const {
		return animator_;
	}

private:
	std::string name_;

protected:
	PointFloat pos_;
	PointFloat scale_;

private:
	bool alive_;

	Animator* animator_;
};

#endif	// __object_hpp