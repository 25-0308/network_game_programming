#ifndef __background_hpp
#define __background_hpp

#include "common.hpp"
#include "Object.hpp"
#include "Image.hpp"

class Background : public Object {
public:
	Background( const std::string& key, const std::string& fileName, bool premultipliedAlpha );
	virtual ~Background( ) { }

	virtual void update( ) override {
		componentUpdate( );
	}
	virtual void render( HDC hdc ) override;

private:
	Image* image_;
};

#endif	// __background_hpp