#include "../include/Stopwatch.hpp"
#include "../include/ResourceHandler.hpp"
#include "../include/Timer.hpp"
#include "../include/Camera.hpp"

Stopwatch::Stopwatch( PointFloat pos )
	: Object( "Stopwatch", pos, PointFloat( ) ),
	digits( ) {
	digits.reserve( 10 );
	for ( auto i = 0; i < 10; ++i ) {
		auto fileName = std::to_string( i ) + ".png";
		auto digitImage = ResourceHandler::loadImage( fileName, "assets/" + fileName, true );

		assert( digitImage != nullptr && "Failed to load digit image" );

		digits.emplace_back( digitImage );
	}
}

void Stopwatch::render( HDC hdc ) {
	const auto time = Timer::remainTime( );
	auto minutes = time / 60;
	auto seconds = time % 60;

	auto relativePos = pos_ - Camera::getPos( );

	auto minutesPos = PointFloat( relativePos.x - 60.f, relativePos.y );
	auto secondsPos1 = PointFloat( relativePos.x + 20.f, relativePos.y );
	auto secondsPos2 = PointFloat( relativePos.x + 65.f, relativePos.y );

	digits[ minutes ]->draw( hdc, minutesPos );
	digits[ seconds / 10 ]->draw( hdc, secondsPos1 );
	digits[ seconds % 10 ]->draw( hdc, secondsPos2 );
}
