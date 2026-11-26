#ifndef __game_hpp
#define __game_hpp

#include "common.hpp"
#include "Protocol.hpp"

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

	// 자기 플레이어 키(A, D, W, S)만 모아서 바뀌었을 때 C2S_Input 전송
	static void sendInput( );

	// 내 캐릭터와 준비 상태를 지금 씬에서 구해서 바뀌었을 때 C2S_SelectCharacter 전송
	// 캐릭터 선택 화면: 고르는 중인 캐릭터, 준비 안 함
	// 스테이지 선택 화면: 확정한 캐릭터, 준비 안 함
	// 경기 화면: 확정한 캐릭터, 준비 (둘 다 준비하면 서버가 카운트다운 후 경기 시작)
	static void sendSelection( );

	// 받은 패킷 하나 처리
	static void handlePacket( const std::vector<char>& packet );

	// S2C_GameState의 두 플레이어 상태를 InGame 씬의 Player에 적용
	static void applyPlayerStates( const Protocol::S2C_GameState& state );

	static HWND hWnd_;
	static HDC hDC_;
	static POINT rsl_;

	static std::uint16_t clientWidth_;
	static std::uint16_t clientHeight_;

	static HDC memDC_;
	static HBITMAP memBmp_;

	static std::uint8_t sentKeys_;	// 마지막으로 보낸 C2S_Input::keys

	// 서버가 알고 있는 내 선택 (보낼 때 갱신, S2C_CharacterSelected로 서버 값에 다시 맞춤)
	static Protocol::Character sentCharacter_;
	static bool sentReady_;
};

#endif	// __game_hpp
