#ifndef __common_hpp
#define __common_hpp

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

// WinSock2.h는 Windows.h보다 먼저 포함
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include <cstdint>
#include <array>
#include <iostream>

#include "Protocol.hpp"
#include "GameConstants.hpp"

// 수신 스레드가 recv 전에 한 번에 기다리는 시간 (ms)
// 이 간격마다 멈추라는 요청(끊기, 서버 종료)을 확인하므로, 스레드는 이 시간 안에 끝남
constexpr long recvPollMs = 200;

// 송신 버퍼가 차서 막힐 때 기다리는 최대 시간 (ms)
// 클라가 읽지 않아 버퍼가 찼을 때만 기다림, 넘으면 그 플레이어를 끊음
constexpr DWORD sendTimeoutMs = 100;

// 플레이어 작업큐에 쌓일 수 있는 최대 개수 (정상 게임은 틱당 1~2개, 플레이어당 최대 약 74KB)
// 넘으면 패킷을 너무 많이 보내는 것으로 보고 그 플레이어를 끊음
// 클라당 초당 1만 개(틱당 약 170개)가 몇 틱 밀려도 버틸 만큼 여유를 둠
constexpr std::size_t maxQueuedJobs = 1024;

#endif	// __common_hpp
