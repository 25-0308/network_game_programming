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
	inputKeys_[ playerNum - 1 ] = 0;

	if ( SessionManager::count( ) == Protocol::maxPlayers ) {
		std::cout << "2명 접속 완료\n";
	}
}

void GameLogic::onPlayerLeave( std::uint8_t playerNum ) {
	// 전송 실패로 이미 정리된 자리
	if ( !SessionManager::isConnected( playerNum ) ) {
		return;
	}

	SessionManager::remove( playerNum );
	inputKeys_[ playerNum - 1 ] = 0;
	std::cout << "플레이어 " << static_cast<int>( playerNum ) << " 퇴장 ("
		<< SessionManager::count( ) << "/" << Protocol::maxPlayers << ")\n";

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
		inputKeys_[ playerNum - 1 ] = input->keys & Protocol::inputKeyMask;
		std::cout << "플레이어 " << static_cast<int>( playerNum ) << " 입력: "
			<< static_cast<int>( inputKeys_[ playerNum - 1 ] ) << "\n";
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

void GameLogic::update( ) {
	Timer::update( );
}

void GameLogic::render( ) {
	auto frame = Protocol::S2C_Frame{ };
	frame.frame = Timer::frame( );
	frame.dt = Timer::fdt( );
	frame.fps = Timer::fps( );
	frame.remainTime = Timer::remainTime( );
	frame.finished = Timer::isFinished( ) ? 1 : 0;
	SessionManager::broadcast( &frame, sizeof( frame ) );
}

JobQueue GameLogic::jobQueue_;
double GameLogic::nextTick_;
std::array<std::uint8_t, Protocol::maxPlayers> GameLogic::inputKeys_;
