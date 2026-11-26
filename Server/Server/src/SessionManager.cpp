#include "../include/SessionManager.hpp"
#include "../include/Network.hpp"

void SessionManager::init( ) {
	InitializeCriticalSection( &lock_ );
}

std::uint8_t SessionManager::reserve( SOCKET sock ) {
	std::uint8_t playerNum = 0;

	EnterCriticalSection( &lock_ );
	for ( auto i = 0; i < Protocol::maxPlayers; ++i ) {
		if ( sessions_[ i ].state == SlotState::Empty ) {
			sessions_[ i ] = Session{ };
			sessions_[ i ].sock = sock;
			sessions_[ i ].state = SlotState::Reserved;
			playerNum = static_cast<std::uint8_t>( i + 1 );
			break;
		}
	}
	LeaveCriticalSection( &lock_ );

	return playerNum;
}

void SessionManager::activate( std::uint8_t playerNum, HANDLE thread ) {
	EnterCriticalSection( &lock_ );
	auto& session = sessions_[ playerNum - 1 ];
	session.thread = thread;
	session.state = SlotState::Connected;
	LeaveCriticalSection( &lock_ );
}

void SessionManager::cancel( std::uint8_t playerNum ) {
	EnterCriticalSection( &lock_ );
	sessions_[ playerNum - 1 ] = Session{ };
	LeaveCriticalSection( &lock_ );
}

void SessionManager::remove( std::uint8_t playerNum ) {
	EnterCriticalSection( &lock_ );
	auto session = sessions_[ playerNum - 1 ];
	LeaveCriticalSection( &lock_ );

	if ( session.state != SlotState::Connected ) {
		return;
	}

	// PlayerLeave는 수신 스레드가 마지막에 넣으므로 곧 끝남 (락 밖에서 기다림)
	// 끝난 뒤에 자리를 비워야 새 접속이 이 자리에 들어와도 예전 스레드와 섞이지 않음
	WaitForSingleObject( session.thread, INFINITE );
	CloseHandle( session.thread );
	closesocket( session.sock );

	EnterCriticalSection( &lock_ );
	sessions_[ playerNum - 1 ] = Session{ };
	LeaveCriticalSection( &lock_ );
}

void SessionManager::closeAll( ) {
	// 모든 수신 스레드에 멈추라고 요청하고, 기다릴 대상을 복사
	auto sessions = std::array<Session, Protocol::maxPlayers>{ };
	EnterCriticalSection( &lock_ );
	for ( auto& session : sessions_ ) {
		session.stopRequested = true;
	}
	sessions = sessions_;
	LeaveCriticalSection( &lock_ );

	// 수신 스레드는 recvPollMs 안에 끝남
	for ( const auto& session : sessions ) {
		if ( session.state != SlotState::Connected ) {
			continue;
		}
		WaitForSingleObject( session.thread, INFINITE );
		CloseHandle( session.thread );
		closesocket( session.sock );
	}

	EnterCriticalSection( &lock_ );
	sessions_.fill( Session{ } );
	LeaveCriticalSection( &lock_ );
	DeleteCriticalSection( &lock_ );
}

SOCKET SessionManager::getSocket( std::uint8_t playerNum ) {
	EnterCriticalSection( &lock_ );
	auto sock = sessions_[ playerNum - 1 ].sock;
	LeaveCriticalSection( &lock_ );
	return sock;
}

int SessionManager::count( ) {
	int cnt = 0;

	EnterCriticalSection( &lock_ );
	for ( const auto& session : sessions_ ) {
		if ( session.state == SlotState::Connected ) {
			++cnt;
		}
	}
	LeaveCriticalSection( &lock_ );

	return cnt;
}

bool SessionManager::shouldStop( std::uint8_t playerNum ) {
	EnterCriticalSection( &lock_ );
	bool stop = sessions_[ playerNum - 1 ].stopRequested;
	LeaveCriticalSection( &lock_ );
	return stop;
}

void SessionManager::broadcast( const void* packet, int size ) {
	// 보낼 소켓만 락 안에서 복사하고, send는 락 밖에서 함
	auto socks = std::array<SOCKET, Protocol::maxPlayers>{ };
	EnterCriticalSection( &lock_ );
	for ( auto i = 0; i < Protocol::maxPlayers; ++i ) {
		const auto& session = sessions_[ i ];
		bool target = session.state == SlotState::Connected && !session.sendFailed;
		socks[ i ] = target ? session.sock : INVALID_SOCKET;
	}
	LeaveCriticalSection( &lock_ );

	for ( auto i = 0; i < Protocol::maxPlayers; ++i ) {
		if ( socks[ i ] == INVALID_SOCKET || Network::sendPacket( socks[ i ], packet, size ) ) {
			continue;
		}

		// 끊겼거나 클라가 읽지 않아 타임아웃
		// 보내다 만 패킷이 있을 수 있으므로 이 연결은 더 쓰지 않고, 수신 스레드를 끝내서 정리함
		std::cout << "플레이어 " << i + 1 << " 전송 실패, 연결 종료\n";
		EnterCriticalSection( &lock_ );
		sessions_[ i ].sendFailed = true;
		sessions_[ i ].stopRequested = true;
		LeaveCriticalSection( &lock_ );
	}
}

bool SessionManager::takeDropped( std::uint8_t playerNum ) {
	EnterCriticalSection( &lock_ );
	auto& session = sessions_[ playerNum - 1 ];
	bool dropped = session.state == SlotState::Connected && session.sendFailed && !session.dropReported;
	if ( dropped ) {
		session.dropReported = true;
	}
	LeaveCriticalSection( &lock_ );
	return dropped;
}

CRITICAL_SECTION SessionManager::lock_;
std::array<Session, Protocol::maxPlayers> SessionManager::sessions_;
