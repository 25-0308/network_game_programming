#include "../include/ResourceHandler.hpp"

Image* ResourceHandler::loadImage( const std::string& key, const std::string& filePath, bool premultipliedAlpha ) {
	auto img = findImage( key );
	if ( img ) {
		return img;
	}
	
	img = new Image( key, filePath, premultipliedAlpha );
	images_.insert( { key, img } );

	return img;
}

Image* ResourceHandler::findImage( const std::string& key ) {
	auto img = images_.find( key );

	if ( img == images_.end( ) )
		return nullptr;

	assert( img->second != nullptr );
	assert( dynamic_cast<Image*>( img->second ) != nullptr );

	return static_cast<Image*>( img->second );
}

std::map<std::string, Image*> ResourceHandler::images_;
