#ifndef __tick_clock_hpp
#define __tick_clock_hpp

#include <chrono>

// 고정 dt 틱 시계 (accumulator 방식)
// 실제로 흐른 시간을 모아 두고 GameConst::tickDt만큼씩 꺼내 쓰므로
// 경기는 항상 같은 dt로, 평균 tickRate번/초 진행됨
class TickClock {
public:
	using Clock = std::chrono::steady_clock;

	// 한 번에 따라잡는 최대 틱 수
	// 디버거 정지 등으로 크게 밀렸을 때 수백 틱을 한꺼번에 돌리지 않고 남은 시간은 버림
	static constexpr int maxCatchUpTicks = 5;

	// 경기 시작 시 호출, 쌓인 시간을 비움
	void reset( );

	// 지난 호출 이후 흐른 시간으로 이번에 돌릴 틱 수를 구함 (0 ~ maxCatchUpTicks)
	int consumeTicks( );

	// 다음 틱 시각까지 대기
	void sleepUntilNextTick( ) const;

private:
	static constexpr Clock::duration tickDuration( );

	Clock::time_point prev_;
	Clock::duration acc_{ };	// 아직 틱으로 쓰지 않은 시간
};

#endif	// __tick_clock_hpp
