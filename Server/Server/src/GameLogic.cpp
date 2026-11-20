#include "../include/GameLogic.hpp"
#include "../include/SessionManager.hpp"
#include "../include/Timer.hpp"

namespace {
	constexpr double tickTime = 1.0 / GameConst::tickRate;

	// 크기가 맞으면 패킷 구조체로 봄, 아니면 nullptr
	template<class Packet>
	const Packet* asPacket( const char* data, int size ) {
		if ( size != sizeof( Packet ) ) {
			return nullptr;
		}
		return reinterpret_cast<const Packet*>( data );
	}
}

void GameLogic::init( ) {
	Timer::init( );
	nextTick_ = tickTime;
}

void GameLogic::pushJob( const Job& job ) {
	jobQueue_.push( job );
}

double GameLogic::timeUntilNextTick( ) {
	return nextTick_ - Timer::now( );
}

void GameLogic::tick( ) {
	processJobs( );
	update( );
	render( );

	// 정해진 간격으로 예약해서 평균 GameConst::tickRate 유지, 많이 밀리면 지금부터 다시 시작
	nextTick_ += tickTime;
	if ( Timer::now( ) - nextTick_ > 0.25 ) {
		nextTick_ = Timer::now( ) + tickTime;
	}
}

void GameLogic::processJobs( ) {
	auto job = Job{ };
	while ( jobQueue_.pop( job ) ) {
		processJob( job );
	}
}

void GameLogic::processJob( const Job& job ) {
	switch ( job.type ) {
	case JobType::PlayerJoin:
		onPlayerJoin( job.playerNum );
		break;
	case JobType::PlayerLeave:
		onPlayerLeave( job.playerNum );
		break;
	case JobType::Packet:
		onPacket( job.playerNum, job.data.data( ), job.size );
		break;
	}
}

void GameLogic::onPlayerJoin( std::uint8_t playerNum ) {
	characters_[ playerNum - 1 ] = Protocol::Character::America;

	if ( SessionManager::count( ) == Protocol::maxPlayers && !playing_ ) {
		std::cout << "2명 접속 완료\n";
		startMatch( );
	}
}

void GameLogic::onPlayerLeave( std::uint8_t playerNum ) {
	// 전송 실패로 이미 정리된 자리
	if ( !SessionManager::isConnected( playerNum ) ) {
		return;
	}

	SessionManager::remove( playerNum );
	std::cout << "플레이어 " << static_cast<int>( playerNum ) << " 퇴장 ("
		<< SessionManager::count( ) << "/" << Protocol::maxPlayers << ")\n";

	// 혼자서는 경기를 이어갈 수 없으므로 중단, 다시 2명이 되면 새로 시작
	if ( playing_ ) {
		playing_ = false;
		Timer::stopMatch( );
		std::cout << "경기 중단\n";
	}

	// 남은 플레이어에게 알림
	auto enemyLeft = Protocol::S2C_EnemyLeft{ };
	SessionManager::broadcast( &enemyLeft, sizeof( enemyLeft ) );
}

void GameLogic::onPacket( std::uint8_t playerNum, const char* data, int size ) {
	auto header = reinterpret_cast<const Protocol::PacketHeader*>( data );

	switch ( header->type ) {
	case Protocol::PacketType::C2S_SelectCharacter: {
		auto select = asPacket<Protocol::C2S_SelectCharacter>( data, size );
		if ( select == nullptr || static_cast<std::size_t>( select->character ) >= GameConst::characterStats.size( ) ) {
			break;
		}

		// 확정한 캐릭터는 다음 경기 시작 때 사용
		if ( select->confirmed ) {
			characters_[ playerNum - 1 ] = select->character;
		}

		// 상대 화면에도 보이도록 두 명 모두에게 전달
		auto selected = Protocol::S2C_CharacterSelected{ };
		selected.playerNum = playerNum;
		selected.character = select->character;
		selected.confirmed = select->confirmed;
		SessionManager::broadcast( &selected, sizeof( selected ) );
		return;
	}
	case Protocol::PacketType::C2S_Input: {
		auto input = asPacket<Protocol::C2S_Input>( data, size );
		if ( input == nullptr ) {
			break;
		}
		// 경기 중이 아니면 버림 (경기 시작 때 키 상태를 비우고 시작)
		if ( playing_ ) {
			match_.applyInput( playerNum, input->keys );
		}
		return;
	}
	case Protocol::PacketType::C2S_Rematch: {
		if ( asPacket<Protocol::C2S_Rematch>( data, size ) == nullptr ) {
			break;
		}
		std::cout << "플레이어 " << static_cast<int>( playerNum ) << " 재시작 요청\n";
		return;
	}
	default:
		break;
	}

	std::cout << "잘못된 패킷 (플레이어 " << static_cast<int>( playerNum ) << ", 종류 "
		<< static_cast<int>( header->type ) << ", 크기 " << size << ")\n";
}

void GameLogic::startMatch( ) {
	// Match::start가 플레이어 초기화와 Timer::startMatch를 함께 함
	match_.start( characters_[ 0 ], characters_[ 1 ] );
	scores_.fill( 0 );
	playing_ = true;
	ended_ = false;
	std::cout << "경기 시작\n";
}

void GameLogic::update( ) {
	Timer::update( );

	if ( !playing_ ) {
		return;
	}

	// 시간이 끝난 틱에는 진행하지 않고 종료
	if ( Timer::isFinished( ) ) {
		playing_ = false;
		ended_ = true;
		std::cout << "경기 종료 (" << static_cast<int>( scores_[ 0 ] ) << " : "
			<< static_cast<int>( scores_[ 1 ] ) << ")\n";
		return;
	}

	match_.tick( );
}

void GameLogic::render( ) {
	auto frame = Protocol::S2C_Frame{ };
	frame.frame = Timer::frame( );
	frame.dt = Timer::fdt( );
	frame.fps = Timer::fps( );
	frame.remainTime = Timer::remainTime( );
	frame.finished = Timer::isFinished( ) ? 1 : 0;
	SessionManager::broadcast( &frame, sizeof( frame ) );

	if ( playing_ ) {
		auto state = Protocol::S2C_GameState{ };
		for ( auto i = 0; i < Protocol::maxPlayers; ++i ) {
			state.players[ i ] = match_.player( static_cast<std::uint8_t>( i + 1 ) ).toState( );
			state.scores[ i ] = scores_[ i ];
		}
		state.remainTime = Timer::remainTime( );
		SessionManager::broadcast( &state, sizeof( state ) );
	}

	if ( ended_ ) {
		ended_ = false;

		auto gameEnd = Protocol::S2C_GameEnd{ };
		for ( auto i = 0; i < Protocol::maxPlayers; ++i ) {
			gameEnd.scores[ i ] = scores_[ i ];
		}
		SessionManager::broadcast( &gameEnd, sizeof( gameEnd ) );
	}
}

JobQueue GameLogic::jobQueue_;
double GameLogic::nextTick_;

Match GameLogic::match_;
bool GameLogic::playing_ = false;
bool GameLogic::ended_ = false;

std::array<Protocol::Character, Protocol::maxPlayers> GameLogic::characters_{ };
std::array<std::uint8_t, Protocol::maxPlayers> GameLogic::scores_{ };
