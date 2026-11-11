#include "../include/SessionManager.hpp"
#include "../include/Network.hpp"

std::uint8_t SessionManager::add( SOCKET sock ) {
	for ( auto i = 0; i < Protocol::maxPlayers; ++i ) {
		if ( !sessions_[ i ].connected ) {
			sessions_[ i ].sock = sock;
			sessions_[ i ].connected = true;
			return static_cast<std::uint8_t>( i + 1 );
		}
	}
	return 0;
}

void SessionManager::remove( std::uint8_t playerNum ) {
	auto& session = sessions_[ playerNum - 1 ];
	if ( !session.connected ) {
		return;
	}

	closesocket( session.sock );
	session.sock = INVALID_SOCKET;
	session.connected = false;
}

SOCKET SessionManager::getSocket( std::uint8_t playerNum ) {
	return sessions_[ playerNum - 1 ].sock;
}

bool SessionManager::isConnected( std::uint8_t playerNum ) {
	return sessions_[ playerNum - 1 ].connected;
}

int SessionManager::count( ) {
	int cnt = 0;
	for ( const auto& session : sessions_ ) {
		if ( session.connected ) {
			++cnt;
		}
	}
	return cnt;
}

void SessionManager::broadcast( const void* packet, int size ) {
	for ( const auto& session : sessions_ ) {
		if ( session.connected ) {
			Network::sendPacket( session.sock, packet, size );
		}
	}
}

std::array<Session, Protocol::maxPlayers> SessionManager::sessions_;
