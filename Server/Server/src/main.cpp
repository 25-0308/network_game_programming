#include "../include/common.hpp"
#include "../include/Network.hpp"
#include "../include/SessionManager.hpp"

int main( ) {
	// 콘솔에 한글 출력
	SetConsoleOutputCP( CP_UTF8 );

	if ( !Network::init( ) ) {
		return 1;
	}

	if ( !Network::listen( Protocol::serverPort ) ) {
		Network::release( );
		return 1;
	}

	std::cout << "서버 시작 (포트 " << Protocol::serverPort << ")\n";
	std::cout << "클라이언트 접속 대기\n";

	// 접속 대기 루프
	while ( true ) {
		auto sock = Network::accept( );
		if ( sock == INVALID_SOCKET ) {
			continue;
		}

		auto playerNum = SessionManager::add( sock );

		// 2명이 이미 있으면 세 번째 접속은 거절
		if ( playerNum == 0 ) {
			std::cout << "[거절] 자리가 없음\n";
			closesocket( sock );
			continue;
		}

		auto login = Protocol::S2C_Login{ };
		login.playerNum = playerNum;
		if ( !Network::sendPacket( sock, &login, sizeof( login ) ) ) {
			SessionManager::remove( playerNum );
			continue;
		}

		std::cout << "플레이어 " << static_cast<int>( playerNum ) << " 입장 ("
			<< SessionManager::count( ) << "/" << Protocol::maxPlayers << ")\n";

		if ( SessionManager::count( ) == Protocol::maxPlayers ) {
			std::cout << "2명 접속 완료\n";
		}
	}

	Network::release( );
	return 0;
}
