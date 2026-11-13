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

#endif	// __common_hpp
