#ifndef __timer_hpp
#define __timer_hpp

#include "common.hpp"
#include "Protocol.hpp"

// 서버에서 받은 프레임 정보 (클라이언트는 시간을 계산하지 않음)
class Timer {
public:
	// S2C_Frame 값 적용
	static void setFrame( const Protocol::S2C_Frame& frame );

	// 창 제목에 서버 FPS, DT 표시
	static void render( );

	static double dt( ) {
		return dt_;
	}
	static float fdt( ) {
		return static_cast<float>( dt_ );
	}
	static std::uint32_t frame( ) {
		return frame_;
	}

private:
	static double dt_;
	static std::uint32_t frame_;
	static std::uint16_t fps_;
	static std::uint32_t shownFrame_;	// 창 제목에 마지막으로 표시한 프레임
};

#endif	// __timer_hpp
