#ifndef __game_hpp
#define __game_hpp

#include "common.hpp"

class Game {
public:
	// 서버 접속에 실패하면 false
	static bool init( HWND hWnd, POINT rsl, const wchar_t* serverIp );
	static void start( );
	static void release( );

	static HWND getHwnd( ) {
		return hWnd_;
	}

	static POINT getRsl( ) {
		return rsl_;
	}

private:
	// 서버 프레임 하나만큼 게임 갱신
	static void step( );

	// 받은 패킷 하나 처리
	static void handlePacket( const std::vector<char>& packet );

	static HWND hWnd_;
	static HDC hDC_;
	static POINT rsl_;

	static std::uint16_t clientWidth_;
	static std::uint16_t clientHeight_;

	static HDC memDC_;
	static HBITMAP memBmp_;
};

#endif	// __game_hpp
