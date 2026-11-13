#include "../include/common.hpp"
#include "../include/Network.hpp"
#include "../include/SessionManager.hpp"
#include "../include/Timer.hpp"

#include <timeapi.h>
#pragma comment( lib, "winmm" )

namespace {
	// 새 접속 처리: 자리 배정, S2C_Login 전송
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

		if ( SessionManager::count( ) == Protocol::maxPlayers ) {
			std::cout << "2명 접속 완료\n";
		}
	}

	// 한 프레임 진행 후 클라에 프레임 정보 전송
	void tick( ) {
		Timer::update( );

		auto frame = Protocol::S2C_Frame{ };
		frame.frame = Timer::frame( );
		frame.dt = Timer::fdt( );
		frame.fps = Timer::fps( );
		SessionManager::broadcast( &frame, sizeof( frame ) );
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

	std::cout << "서버 시작 (포트 " << Protocol::serverPort << ", " << tickRate << " FPS)\n";
	std::cout << "클라이언트 접속 대기\n";

	const double tickTime = 1.0 / tickRate;
	Timer::init( );
	double nextTick = tickTime;

	// 다음 틱까지 남은 시간 동안 접속을 기다리고, 시간이 되면 프레임 진행
	while ( true ) {
		double remain = nextTick - Timer::now( );
		if ( remain > 0.0 ) {
			if ( Network::waitAccept( remain ) ) {
				acceptClient( );
			}
			continue;
		}

		tick( );

		// 정해진 간격으로 예약해서 평균 tickRate 유지, 많이 밀리면 지금부터 다시 시작
		nextTick += tickTime;
		if ( Timer::now( ) - nextTick > 0.25 ) {
			nextTick = Timer::now( ) + tickTime;
		}
	}

	Network::release( );
	timeEndPeriod( 1 );
	return 0;
}
