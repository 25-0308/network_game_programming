#ifndef __job_hpp
#define __job_hpp

#include "common.hpp"

enum class JobType : std::uint8_t {
	PlayerJoin,		// 접속 완료 (S2C_Login까지 보낸 뒤)
	PlayerLeave,	// 접속 끊김
	Packet,			// 클라가 보낸 패킷 하나
};

// 전체 작업큐에 들어가는 일 하나
struct Job {
	JobType type;
	std::uint8_t playerNum;	// 1 또는 2
	std::array<char, Protocol::maxPacketSize> data;	// Packet일 때 패킷 전체 (헤더 포함)
	int size;
};

#endif	// __job_hpp
