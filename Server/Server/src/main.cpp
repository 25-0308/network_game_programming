#include "../include/common.hpp"
#include "../include/Network.hpp"
#include "../include/SessionManager.hpp"
#include "../include/GameLogic.hpp"
#include "../include/Threads.hpp"

#include <timeapi.h>
#pragma comment( lib, "winmm" )

namespace {
	// 끊김을 작업큐에 넣은 자리, 처리될 때까지 같은 소켓을 다시 읽지 않음
	std::array<bool, Protocol::maxPlayers> stopped{ };

	// 새 접속 처리: 자리 배정, S2C_Login 전송 후 PlayerJoin을 작업큐에 넣음
	void acceptClient( ) {
		auto sock = Network::accept( );
		if ( sock == INVALID_SOCKET ) {
			return;
		}

		auto playerNum = SessionManager::add( sock );

		// 2명이 이미 있으면 세 번째 접속은 거절
		if ( playerNum == 0 ) {
			std::cout << "[거절] 자리가 없음\n";
			closesocket( sock );
			return;
		}

		auto login = Protocol::S2C_Login{ };
		login.playerNum = playerNum;
		if ( !Network::sendPacket( sock, &login, sizeof( login ) ) ) {
			SessionManager::remove( playerNum );
			return;
		}

		std::cout << "플레이어 " << static_cast<int>( playerNum ) << " 입장 ("
			<< SessionManager::count( ) << "/" << Protocol::maxPlayers << ")\n";

		stopped[ playerNum - 1 ] = false;

		auto job = Job{ };
		job.type = JobType::PlayerJoin;
		job.playerNum = playerNum;
		GameLogic::pushJob( job );
	}

	// 최대 seconds 동안 접속과 수신을 받아 작업큐에 넣음
	// 수신 스레드가 생기면 플레이어 수신은 player_1_thread, player_2_thread가 대신함
	void receive( double seconds ) {
		auto socks = std::array<SOCKET, Protocol::maxPlayers>{ };
		for ( auto i = 0; i < Protocol::maxPlayers; ++i ) {
			auto playerNum = static_cast<std::uint8_t>( i + 1 );
			bool reading = SessionManager::isConnected( playerNum ) && !stopped[ i ];
			socks[ i ] = reading ? SessionManager::getSocket( playerNum ) : INVALID_SOCKET;
		}

		bool listenReady = false;
		auto ready = std::array<bool, Protocol::maxPlayers>{ };
		Network::waitReadable( seconds, socks, listenReady, ready );

		// 데이터가 온 뒤에 읽으므로 패킷 하나를 다 받을 때까지만 잠깐 막힘
		for ( auto i = 0; i < Protocol::maxPlayers; ++i ) {
			if ( ready[ i ] && !receivePlayerPacket( static_cast<std::uint8_t>( i + 1 ) ) ) {
				stopped[ i ] = true;
			}
		}

		if ( listenReady ) {
			acceptClient( );
		}
	}
}

int main( ) {
	// 콘솔에 한글 출력
	SetConsoleOutputCP( CP_UTF8 );

	// 대기 시간을 1ms 단위로 (틱 간격용)
	timeBeginPeriod( 1 );

	if ( !Network::init( ) ) {
		return 1;
	}

	if ( !Network::listen( Protocol::serverPort ) ) {
		Network::release( );
		return 1;
	}

	std::cout << "서버 시작 (포트 " << Protocol::serverPort << ", " << GameConst::tickRate << " FPS)\n";
	std::cout << "클라이언트 접속 대기\n";

	GameLogic::init( );

	// 다음 틱까지는 접속과 수신을 받아 작업큐에 쌓고, 시간이 되면 작업큐 실행 후 프레임 진행
	while ( true ) {
		auto remain = GameLogic::timeUntilNextTick( );
		if ( remain > 0.0 ) {
			receive( remain );
			continue;
		}
		GameLogic::tick( );
	}

	Network::release( );
	timeEndPeriod( 1 );
	return 0;
}
