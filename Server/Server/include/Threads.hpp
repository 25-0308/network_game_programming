#ifndef __threads_hpp
#define __threads_hpp

#include "common.hpp"

// 지금은 메인 스레드 하나로 돌림 (main.cpp)
// 아래 스레드 함수는 락을 추가한 뒤 CreateThread로 실행

// 플레이어 패킷 하나를 받아 전체 작업큐에 넣음
// 접속이 끊기면 PlayerLeave를 넣고 false
bool receivePlayerPacket( std::uint8_t playerNum );

// 플레이어 1 수신 스레드
DWORD WINAPI player_1_thread( LPVOID arg );

// 플레이어 2 수신 스레드
DWORD WINAPI player_2_thread( LPVOID arg );

// 게임 로직 스레드
DWORD WINAPI game_logic_thread( LPVOID arg );

#endif	// __threads_hpp
