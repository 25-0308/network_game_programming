#ifndef __button_hpp
#define __button_hpp

#include "common.hpp"
#include "Object.hpp"
#include "Image.hpp"

class Button : public Object {
public:
	Button( const std::string& key, const std::string& fileName, bool premultipliedAlpha );
	virtual ~Button( ) {}

	void update( ) override;
	void render( HDC hdc ) override;

	bool mouseOnCheck( );

	void mouseOn( ) {}
	void mouseLbtnUp( ) {}
	void mouseLbtnClicked( );

	void resetImageState( );

private:
	Image* imageOn_;
	Image* imageOff_;

	bool bMouseOn_;
	bool bLbtnDown_;
};

#endif	// __button_hpp