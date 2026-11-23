#include "../include/common.hpp"
#include "../include/Network.hpp"
#include "../include/SessionManager.hpp"
#include "../include/GameLogic.hpp"
#include "../include/Threads.hpp"

#include <timeapi.h>
#pragma comment( lib, "winmm" )

namespace {
	// 새 접속 처리: 자리 예약 -> S2C_Login 전송 -> 등록 -> PlayerJoin -> 수신 스레드 시작
	// S2C_Login을 다 보낸 뒤 등록해야 게임 로직 스레드의 전송과 섞이지 않음
	// PlayerJoin을 넣은 뒤 스레드를 시작해야 그 플레이어 큐에서 Join이 항상 먼저 옴
	void acceptClient( ) {
		auto sock = Network::accept( );
		if ( sock == INVALID_SOCKET ) {
			return;
		}

		auto playerNum = SessionManager::reserve( sock );

		// 2명이 이미 있으면 세 번째 접속은 거절
		if ( playerNum == 0 ) {
			std::cout << "[거절] 자리가 없음\n";
			closesocket( sock );
			return;
		}

		auto login = Protocol::S2C_Login{ };
		login.playerNum = playerNum;
		if ( !Network::sendPacket( sock, &login, sizeof( login ) ) ) {
			SessionManager::cancel( playerNum );
			closesocket( sock );
			return;
		}

		// 핸들을 먼저 등록하고 실행하도록 멈춘 채로 만듦
		auto threadFunc = playerNum == 1 ? player_1_thread : player_2_thread;
		auto thread = CreateThread( nullptr, 0, threadFunc, nullptr, CREATE_SUSPENDED, nullptr );
		if ( thread == nullptr ) {
			Network::printError( "CreateThread" );
			SessionManager::cancel( playerNum );
			closesocket( sock );
			return;
		}

		SessionManager::activate( playerNum, thread );
		std::cout << "플레이어 " << static_cast<int>( playerNum ) << " 입장 ("
			<< SessionManager::count( ) << "/" << Protocol::maxPlayers << ")\n";

		auto job = Job{ };
		job.type = JobType::PlayerJoin;
		job.playerNum = playerNum;
		GameLogic::pushJob( job );

		ResumeThread( thread );
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

	SessionManager::init( );
	GameLogic::init( );

	auto gameThread = CreateThread( nullptr, 0, game_logic_thread, nullptr, 0, nullptr );
	if ( gameThread == nullptr ) {
		Network::printError( "CreateThread" );
		Network::release( );
		return 1;
	}

	// 메인 스레드는 접속만 받음 (블로킹 accept)
	while ( true ) {
		acceptClient( );
	}

	CloseHandle( gameThread );
	Network::release( );
	timeEndPeriod( 1 );
	return 0;
}
