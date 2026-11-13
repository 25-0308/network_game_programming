#include "../include/Timer.hpp"

#include <algorithm>

namespace {
	// 루프가 멈췄다 돌아와도 한 번에 크게 움직이지 않도록 제한
	constexpr double maxDt = 1.0 / 20.0;
}

void Timer::init( ) {
	QueryPerformanceFrequency( &freq_ );
	QueryPerformanceCounter( &prevCnt_ );
	startCnt_ = prevCnt_;
}

void Timer::update( ) {
	QueryPerformanceCounter( &currCnt_ );
	dt_ = static_cast<double>( currCnt_.QuadPart - prevCnt_.QuadPart ) / static_cast<double>( freq_.QuadPart );
	prevCnt_ = currCnt_;

	dt_ = std::min( dt_, maxDt );
	++frame_;

	// 1초마다 FPS 갱신
	++frameCnt_;
	accT_ += dt_;
	if ( accT_ >= 1.0 ) {
		fps_ = frameCnt_;
		frameCnt_ = 0;
		accT_ -= 1.0;
	}
}

double Timer::now( ) {
	LARGE_INTEGER cnt;
	QueryPerformanceCounter( &cnt );
	return static_cast<double>( cnt.QuadPart - startCnt_.QuadPart ) / static_cast<double>( freq_.QuadPart );
}

LARGE_INTEGER Timer::prevCnt_;
LARGE_INTEGER Timer::currCnt_;
LARGE_INTEGER Timer::freq_;
LARGE_INTEGER Timer::startCnt_;

double Timer::dt_;
std::uint32_t Timer::frame_;

double Timer::accT_;
std::uint16_t Timer::frameCnt_;
std::uint16_t Timer::fps_;
