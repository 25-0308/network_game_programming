#ifndef __resource_handler_hpp
#define __resource_handler_hpp

#include "common.hpp"
#include "Image.hpp"

/*
ResourceHandler is responsible for managing game resources such as images and sounds.
*/

class ResourceHandler {
public:
	static Image* loadImage( const std::string& key, const std::string& filePath, bool premultipliedAlpha );
	static Image* findImage( const std::string& key );

	static void release( ) {
		for ( auto [null, image] : images_ ) {
			delete image;
		}
		images_.clear( );
	}

private:
	static std::map<std::string, Image*> images_;
};

#endif	// __resource_handler_hpp