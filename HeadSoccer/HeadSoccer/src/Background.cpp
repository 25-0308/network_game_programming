#include "../include/Background.hpp"
#include "../include/ResourceHandler.hpp"
#include "../include/Camera.hpp"

#include <iostream>

Background::Background( const std::string& key, const std::string& fileName, bool premultipliedAlpha ) {
	// 프로젝트 실행 경로는 Background.cpp 파일이 있는 경로(src 디렉터리의 경로)가 아니라
	// D:\source\repos\2025-winapi-project\HeadSoccer\HeadSoccer -> src 파일에서 한 단계 위의 디렉터리가 된다.
	// 따라서 "../assets/" 경로가 아니라 "assets/" 경로를 사용해야 한다.

	auto filePath = fs::path( "assets/" + fileName );
	image_ = ResourceHandler::loadImage( key, fs::canonical( filePath ).string( ), premultipliedAlpha );
}

void Background::render( HDC hdc ) {
	auto cameraPos = Camera::getPos( );
	auto relativePos = pos_ - cameraPos;

	image_->draw( hdc, relativePos );

	componentRender( hdc );
}
