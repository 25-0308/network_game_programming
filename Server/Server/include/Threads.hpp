#ifndef __threads_hpp
#define __threads_hpp

#include "common.hpp"

// 메인 스레드: 접속을 받아 자리 배정, S2C_Login 전송, 수신 스레드 시작 (main.cpp)
// 플레이어 스레드: 자기 소켓만 읽어 자기 작업큐에만 넣음
// 게임 로직 스레드: 플레이어 작업큐를 가져와 실행하고 전송

// 플레이어 패킷 하나를 받아 그 플레이어 작업큐에 넣음
// 접속이 끊기면 PlayerLeave를 넣고 false (스레드의 마지막 작업)
bool receivePlayerPacket( std::uint8_t playerNum );

// 플레이어 1 수신 스레드 (접속할 때마다 새로 만들고, 끊기면 끝남)
DWORD WINAPI player_1_thread( LPVOID arg );

// 플레이어 2 수신 스레드
DWORD WINAPI player_2_thread( LPVOID arg );

// 게임 로직 스레드 (서버 시작 시 하나)
DWORD WINAPI game_logic_thread( LPVOID arg );

#endif	// __threads_hpp
