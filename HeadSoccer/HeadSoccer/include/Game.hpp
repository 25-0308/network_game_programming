#ifndef __game_hpp
#define __game_hpp

#include "common.hpp"

class Game {
public:
	static void init( HWND hWnd, POINT rsl );
	static void start( );
	static void release( );

	static HWND getHwnd( ) { 
		return hWnd_; 
	}

	static POINT getRsl( ) {
		return rsl_;
	}

private:
	static HWND hWnd_;
	static HDC hDC_;
	static POINT rsl_;

	static std::uint16_t clientWidth_;
	static std::uint16_t clientHeight_;
	
	static HDC memDC_;
	static HBITMAP memBmp_;
};

#endif	// __game_hpp