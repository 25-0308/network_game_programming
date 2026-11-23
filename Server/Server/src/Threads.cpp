#include "../include/Threads.hpp"
#include "../include/GameLogic.hpp"
#include "../include/Network.hpp"
#include "../include/SessionManager.hpp"

namespace {
	// 끊길 때까지 받아서 자기 작업큐에 넣기만 함
	DWORD playerThread( std::uint8_t playerNum ) {
		while ( receivePlayerPacket( playerNum ) ) {
		}
		return 0;
	}
}

bool receivePlayerPacket( std::uint8_t playerNum ) {
	auto job = Job{ };
	job.playerNum = playerNum;

	job.size = Network::recvPacket( SessionManager::getSocket( playerNum ), job.data.data( ), maxPacketSize );
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
	while ( true ) {
		auto remain = GameLogic::timeUntilNextTick( );
		if ( remain > 0.0 ) {
			Sleep( static_cast<DWORD>( remain * 1000.0 ) );
			continue;
		}
		GameLogic::tick( );
	}
	return 0;
}
