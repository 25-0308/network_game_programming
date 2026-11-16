#include "../include/TickClock.hpp"
#include "../include/common.hpp"

#include <thread>

constexpr TickClock::Clock::duration TickClock::tickDuration( ) {
	return std::chrono::duration_cast<Clock::duration>(
		std::chrono::duration<double>( 1.0 / GameConst::tickRate ) );
}

void TickClock::reset( ) {
	prev_ = Clock::now( );
	acc_ = Clock::duration::zero( );
}

int TickClock::consumeTicks( ) {
	auto now = Clock::now( );
	acc_ += now - prev_;
	prev_ = now;

	int ticks = 0;
	while ( acc_ >= tickDuration( ) && ticks < maxCatchUpTicks ) {
		acc_ -= tickDuration( );
		++ticks;
	}

	// 따라잡지 못한 시간은 버림
	if ( acc_ >= tickDuration( ) ) {
		acc_ = Clock::duration::zero( );
	}

	return ticks;
}

void TickClock::sleepUntilNextTick( ) const {
	// 남은 시간 = 한 틱 - 쌓인 시간 - 마지막 consumeTicks 이후 흐른 시간
	auto elapsed = acc_ + ( Clock::now( ) - prev_ );
	if ( elapsed < tickDuration( ) ) {
		std::this_thread::sleep_for( tickDuration( ) - elapsed );
	}
}
