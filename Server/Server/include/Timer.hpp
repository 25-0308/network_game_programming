#ifndef __timer_hpp
#define __timer_hpp

#include "common.hpp"

// 델타 타임, 프레임 번호, FPS를 서버에서 계산 (클라는 S2C_Frame으로 받음)
class Timer {
public:
	static void init( );

	// 한 프레임 진행: dt, 프레임 번호, FPS 갱신
	static void update( );

	// 경기 시작 시 호출한다. 서버 실행 및 접속만으로는 카운트다운하지 않는다.
	static void startMatch( std::uint16_t time = GameConst::matchTime );
	static std::uint16_t remainTime( ) {
		return time_;
	}
	static bool isFinished( ) {
		return finished_;
	}

	// init 이후 지난 시간 (틱 간격 맞추기용)
	static double now( );

	static double dt( ) {
		return dt_;
	}
	static float fdt( ) {
		return static_cast<float>( dt_ );
	}
	static std::uint32_t frame( ) {
		return frame_;
	}
	static std::uint16_t fps( ) {
		return fps_;
	}

private:
	static LARGE_INTEGER prevCnt_;
	static LARGE_INTEGER currCnt_;
	static LARGE_INTEGER freq_;
	static LARGE_INTEGER startCnt_;

	static double dt_;
	static std::uint32_t frame_;

	// FPS 계산용
	static double accT_;
	static std::uint16_t frameCnt_;
	static std::uint16_t fps_;

	static std::uint16_t time_;
	static float matchAccT_;
	static bool running_;
	static bool finished_;
};

#endif	// __timer_hpp
