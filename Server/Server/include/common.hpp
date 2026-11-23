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

// 받을 수 있는 패킷 최대 크기 (가장 큰 패킷보다 넉넉하게)
constexpr int maxPacketSize = 64;

// 송신 버퍼가 차서 막힐 때 기다리는 최대 시간 (ms)
// 클라가 읽지 않아 버퍼가 찼을 때만 기다림, 넘으면 그 플레이어를 끊음
constexpr DWORD sendTimeoutMs = 100;

// 플레이어 작업큐에 쌓일 수 있는 최대 개수 (정상 게임은 틱당 1~2개)
// 넘으면 패킷을 너무 많이 보내는 것으로 보고 그 플레이어를 끊음
constexpr std::size_t maxQueuedJobs = 256;

#endif	// __common_hpp
