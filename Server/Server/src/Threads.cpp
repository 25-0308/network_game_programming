#include "../include/Threads.hpp"
#include "../include/GameLogic.hpp"
#include "../include/Network.hpp"
#include "../include/SessionManager.hpp"

#include <atomic>

namespace {
	std::atomic<bool> running{ true };

	// 끊기거나 멈추라는 요청이 올 때까지 받아서 자기 작업큐에 넣기만 함
	DWORD playerThread( std::uint8_t playerNum ) {
		// 소켓은 이 스레드가 끝난 뒤에만 닫히므로 처음 한 번만 가져옴
		auto sock = SessionManager::getSocket( playerNum );
		while ( receivePlayerPacket( playerNum, sock ) ) {
		}
		return 0;
	}
}

bool isRunning( ) {
	return running;
}

void requestShutdown( ) {
	running = false;
}

bool receivePlayerPacket( std::uint8_t playerNum, SOCKET sock ) {
	auto job = Job{ };
	job.playerNum = playerNum;

	auto shouldStop = [ playerNum ] {
		return !isRunning( ) || SessionManager::shouldStop( playerNum );
	};

	job.size = Network::recvPacket( sock, job.data.data( ), Protocol::maxPacketSize, shouldStop );
	if ( job.size < 0 ) {
		job.type = JobType::PlayerLeave;
		GameLogic::pushJob( job );
		return false;
	}

	job.type = JobType::Packet;
	if ( !GameLogic::pushJob( job ) ) {
		// 게임 로직이 처리하는 속도보다 훨씬 많이 보냄, 더 받지 않고 끊음 (소켓은 게임 로직이 닫음)
		std::cout << "플레이어 " << static_cast<int>( playerNum ) << " 패킷 과다, 연결 종료\n";
		job.type = JobType::PlayerLeave;
		GameLogic::pushJob( job );
		return false;
	}
	return true;
}

DWORD WINAPI player_1_thread( LPVOID arg ) {
	return playerThread( 1 );
}

DWORD WINAPI player_2_thread( LPVOID arg ) {
	return playerThread( 2 );
}

DWORD WINAPI game_logic_thread( LPVOID arg ) {
	// 다음 틱까지 쉬고, 시간이 되면 작업큐 실행 후 프레임 진행
	while ( isRunning( ) ) {
		auto remain = GameLogic::timeUntilNextTick( );
		if ( remain > 0.0 ) {
			Sleep( static_cast<DWORD>( remain * 1000.0 ) );
			continue;
		}
		GameLogic::tick( );
	}
	return 0;
}
