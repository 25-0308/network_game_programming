#include "../include/Threads.hpp"
#include "../include/GameLogic.hpp"
#include "../include/Network.hpp"
#include "../include/SessionManager.hpp"

namespace {
	// 끊길 때까지 받기만 함
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
	GameLogic::pushJob( job );
	return true;
}

DWORD WINAPI player_1_thread( LPVOID arg ) {
	return playerThread( 1 );
}

DWORD WINAPI player_2_thread( LPVOID arg ) {
	return playerThread( 2 );
}

DWORD WINAPI game_logic_thread( LPVOID arg ) {
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
