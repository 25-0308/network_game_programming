#include "../include/Game.hpp"
#include "../include/Timer.hpp"

#include <array>

void Timer::setFrame( const Protocol::S2C_Frame& frame ) {
	dt_ = frame.dt;
	frame_ = frame.frame;
	fps_ = frame.fps;
}

void Timer::render( ) {
	// 새 프레임을 받았을 때만 갱신
	if ( frame_ == shownFrame_ ) {
		return;
	}
	shownFrame_ = frame_;

	auto buffer = std::array<WCHAR, 100>( );
	swprintf_s( buffer.data( ), buffer.size( ), L"FPS: %u, DT: %lf, Frame: %u", fps_, dt_, frame_ );
	SetWindowText( Game::getHwnd( ), buffer.data( ) );
}

double Timer::dt_;
std::uint32_t Timer::frame_;
std::uint16_t Timer::fps_;
std::uint32_t Timer::shownFrame_;
