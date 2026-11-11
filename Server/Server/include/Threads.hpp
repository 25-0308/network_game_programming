#ifndef __threads_hpp
#define __threads_hpp

#include "common.hpp"

// 스레드 이름만 선언 (구현은 추후)

// 플레이어 1 수신 스레드
DWORD WINAPI player_1_thread( LPVOID arg );

// 플레이어 2 수신 스레드
DWORD WINAPI player_2_thread( LPVOID arg );

// 게임 로직 스레드
DWORD WINAPI game_logic_thread( LPVOID arg );

#endif	// __threads_hpp
