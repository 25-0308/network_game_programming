#include "../include/Timer.hpp"

#include <algorithm>

namespace {
	// 루프가 멈췄다 돌아와도 한 번에 크게 움직이지 않도록 제한
	constexpr double maxDt = 1.0 / 20.0;
}

void Timer::init( ) {
	QueryPerformanceFrequency( &freq_ );
	QueryPerformanceCounter( &prevCnt_ );
}

void Timer::update( ) {
	QueryPerformanceCounter( &currCnt_ );
	dt_ = static_cast<double>( currCnt_.QuadPart - prevCnt_.QuadPart ) / static_cast<double>( freq_.QuadPart );
	prevCnt_ = currCnt_;

	dt_ = std::min( dt_, maxDt );
}

LARGE_INTEGER Timer::prevCnt_;
LARGE_INTEGER Timer::currCnt_;
LARGE_INTEGER Timer::freq_;

double Timer::dt_;
