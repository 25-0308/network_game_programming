#include "../include/Stopwatch.hpp"
#include "../include/ResourceHandler.hpp"
#include "../include/Timer.hpp"
#include "../include/Camera.hpp"
#include "../include/EventHandler.hpp"
#include "../include/SceneHandler.hpp"

Stopwatch::Stopwatch( PointFloat pos, std::uint16_t time )
	: Object( "Stopwatch", pos, PointFloat( ) ),
	digits( ), time_{ time } {
	digits.reserve( 10 );
	for ( auto i = 0; i < 10; ++i ) {
		auto fileName = std::to_string( i ) + ".png";
		auto digitImage = ResourceHandler::loadImage( fileName, "assets/" + fileName, true );

		assert( digitImage != nullptr && "Failed to load digit image" );

		digits.emplace_back( digitImage );
	}
}

void Stopwatch::update( ) {
	time_ = Timer::remainTime( );
	if ( Timer::isFinished( ) && SceneHandler::getCurrSceneType( ) == SceneType::InGame ) {
		EventHandler::addEvent(
			Event{
				.type = EventType::ChangeScene,
				.wParam = static_cast<DWORD_PTR>( SceneType::Result )
			}
		);
	}
}

void Stopwatch::render( HDC hdc ) {
	auto minutes = time_ / 60;
	auto seconds = time_ % 60;

	auto relativePos = pos_ - Camera::getPos( );

	auto minutesPos = PointFloat( relativePos.x - 60.f, relativePos.y );
	auto secondsPos1 = PointFloat( relativePos.x + 20.f, relativePos.y );
	auto secondsPos2 = PointFloat( relativePos.x + 65.f, relativePos.y );

	digits[ minutes ]->draw( hdc, minutesPos );
	digits[ seconds / 10 ]->draw( hdc, secondsPos1 );
	digits[ seconds % 10 ]->draw( hdc, secondsPos2 );
}
