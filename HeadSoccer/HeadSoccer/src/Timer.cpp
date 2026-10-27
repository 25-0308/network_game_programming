#include "../include/Game.hpp"
#include "../include/Timer.hpp"

#include <array>

namespace {
	constexpr double maxDt = 1.0 / 20.0;
}

void Timer::init() {
	QueryPerformanceFrequency(&freq_);
	QueryPerformanceCounter(&prevCnt_);
}

void Timer::update() {
	QueryPerformanceCounter(&currCnt_);
	dt_ = static_cast<double>(currCnt_.QuadPart - prevCnt_.QuadPart) / static_cast<double>(freq_.QuadPart);
	prevCnt_ = currCnt_;

	// dragging the window stalls the loop; one huge step would push the ball through walls and the crossbar
	dt_ = std::min(dt_, maxDt);
}

void Timer::render( ) {
	++fps_;
	accT_ += dt_;
	if (accT_ >= 1.f) {
		auto buffer = std::array<WCHAR, 100>();
		swprintf_s(buffer.data(), buffer.size(), L"FPS: %d, DT: %lf", fps_, dt_);
		SetWindowText( Game::getHwnd( ), buffer.data( ) );

		fps_ = 0;
		accT_ = 0.f;
	}
}

LARGE_INTEGER Timer::prevCnt_;
LARGE_INTEGER Timer::currCnt_;
LARGE_INTEGER Timer::freq_;

double Timer::dt_;
double Timer::accT_;
std::int32_t Timer::fps_;