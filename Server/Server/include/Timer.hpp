#ifndef __timer_hpp
#define __timer_hpp

#include "common.hpp"

// 델타 타임 계산 (클라이언트 Timer를 서버로 옮김, 화면 출력 부분 제외)
class Timer {
public:
	static void init( );

	// 이전 update 이후 지난 시간 계산
	static void update( );

	static double dt( ) {
		return dt_;
	}
	static float fdt( ) {
		return static_cast<float>( dt_ );
	}

private:
	static LARGE_INTEGER prevCnt_;
	static LARGE_INTEGER currCnt_;
	static LARGE_INTEGER freq_;

	static double dt_;
};

#endif	// __timer_hpp
