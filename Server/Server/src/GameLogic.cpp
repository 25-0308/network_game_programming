#include "../include/GameLogic.hpp"
#include "../include/SessionManager.hpp"
#include "../include/Timer.hpp"

#include <random>
#include <cmath>

namespace {
	constexpr double tickTime = 1.0 / GameConst::tickRate;

	std::mt19937 rng;

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
	rng.seed( std::random_device{ }( ) );
}

bool GameLogic::pushJob( const Job& job ) {
	auto& queue = playerQueues_[ job.playerNum - 1 ];
	if ( job.type == JobType::Packet ) {
		return queue.tryPush( job, maxQueuedJobs );
	}
	queue.push( job );
	return true;
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
	// 지난 틱에 전송이 실패한 플레이어는 게임에서 먼저 뺌 (자리는 PlayerLeave에서 정리)
	for ( std::uint8_t num = 1; num <= Protocol::maxPlayers; ++num ) {
		if ( SessionManager::takeDropped( num ) ) {
			dropPlayer( num );
		}
	}

	// 한 번에 플레이어 큐 락 하나만 잡음 (1P -> 2P)
	for ( auto& queue : playerQueues_ ) {
		queue.takeAll( jobs_ );
	}

	while ( !jobs_.empty( ) ) {
		processJob( jobs_.front( ) );
		jobs_.pop( );
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
	joined_[ playerNum - 1 ] = true;
	characters_[ playerNum - 1 ] = Protocol::Character::America;
	ready_[ playerNum - 1 ] = false;

	// 세션 수가 아니라 Join을 처리한 수로 셈
	// 두 명이 거의 동시에 접속하면 상대가 세션에는 있어도 Join은 아직 큐에 남아 있을 수 있음
	if ( joinedCount( ) < Protocol::maxPlayers ) {
		std::cout << "상대 접속 대기\n";
		return;
	}

	// 2명이 모이면 캐릭터 선택, 서로의 현재 선택을 알려 줌
	phase_ = Phase::Select;
	std::cout << "2명 접속 완료, 캐릭터 선택\n";

	auto changeScene = Protocol::S2C_ChangeScene{ };
	SessionManager::broadcast( &changeScene, sizeof( changeScene ) );
	for ( std::uint8_t num = 1; num <= Protocol::maxPlayers; ++num ) {
		sendSelection( num );
	}
}

void GameLogic::onPlayerLeave( std::uint8_t playerNum ) {
	// 수신 스레드가 끝났으므로 자리 정리 (전송 실패로 게임에서 먼저 뺀 경우도 여기서 정리)
	SessionManager::remove( playerNum );
	dropPlayer( playerNum );
}

void GameLogic::dropPlayer( std::uint8_t playerNum ) {
	auto i = playerNum - 1;
	if ( !joined_[ i ] ) {
		return;
	}

	// 다음 사람이 이 자리에 들어와도 예전 선택이 보이지 않게 비움
	joined_[ i ] = false;
	characters_[ i ] = Protocol::Character::America;
	std::cout << "플레이어 " << static_cast<int>( playerNum ) << " 퇴장 ("
		<< joinedCount( ) << "/" << Protocol::maxPlayers << ")\n";

	// 혼자서는 이어갈 수 없으므로 경기, 카운트다운을 멈추고 대기로
	if ( phase_ == Phase::Playing ) {
		Timer::stopMatch( );
		std::cout << "경기 중단\n";
	}
	phase_ = Phase::Waiting;
	ready_.fill( false );

	// 남은 플레이어에게 알림 (준비도 풀림)
	auto enemyLeft = Protocol::S2C_EnemyLeft{ };
	SessionManager::broadcast( &enemyLeft, sizeof( enemyLeft ) );
	for ( std::uint8_t num = 1; num <= Protocol::maxPlayers; ++num ) {
		if ( joined_[ num - 1 ] ) {
			sendSelection( num );
		}
	}
}

int GameLogic::joinedCount( ) {
	int cnt = 0;
	for ( bool joined : joined_ ) {
		if ( joined ) {
			++cnt;
		}
	}
	return cnt;
}

void GameLogic::onPacket( std::uint8_t playerNum, const char* data, int size ) {
	// 게임에서 뺀 플레이어가 PlayerLeave 전까지 보내는 패킷은 버림
	if ( !joined_[ playerNum - 1 ] ) {
		return;
	}

	auto header = reinterpret_cast<const Protocol::PacketHeader*>( data );

	switch ( header->type ) {
	case Protocol::PacketType::C2S_SelectCharacter: {
		auto select = asPacket<Protocol::C2S_SelectCharacter>( data, size );
		if ( select == nullptr || static_cast<std::size_t>( select->character ) >= GameConst::characterStats.size( ) ) {
			break;
		}
		onSelectCharacter( playerNum, *select );
		return;
	}
	case Protocol::PacketType::C2S_Input: {
		auto input = asPacket<Protocol::C2S_Input>( data, size );
		if ( input == nullptr ) {
			break;
		}
		// 경기 중이 아니면 버림 (경기 시작 때 키 상태를 비우고 시작)
		if ( phase_ == Phase::Playing ) {
			match_.applyInput( playerNum, input->keys );
		}
		return;
	}
	case Protocol::PacketType::C2S_Rematch: {
		if ( asPacket<Protocol::C2S_Rematch>( data, size ) == nullptr ) {
			break;
		}
		// 경기가 끝나면 선택 단계로 돌아가므로, 다시 준비하면 재경기
		std::cout << "플레이어 " << static_cast<int>( playerNum ) << " 재시작 요청\n";
		return;
	}
	default:
		break;
	}

	std::cout << "잘못된 패킷 (플레이어 " << static_cast<int>( playerNum ) << ", 종류 "
		<< static_cast<int>( header->type ) << ", 크기 " << size << ")\n";
}

void GameLogic::onSelectCharacter( std::uint8_t playerNum, const Protocol::C2S_SelectCharacter& select ) {
	// 경기 중에는 바꿀 수 없음
	if ( phase_ == Phase::Playing ) {
		return;
	}

	auto i = playerNum - 1;
	bool ready = select.confirmed != 0;
	bool changed = characters_[ i ] != select.character || ready_[ i ] != ready;
	characters_[ i ] = select.character;
	ready_[ i ] = ready;

	if ( !changed ) {
		return;
	}
	sendSelection( playerNum );

	// 카운트다운 중에 취소하거나 바꾸면 다시 선택 단계로
	if ( phase_ == Phase::Countdown ) {
		phase_ = Phase::Select;
		std::cout << "카운트다운 취소\n";
	}
	tryStartCountdown( );
}

void GameLogic::tryStartCountdown( ) {
	if ( phase_ != Phase::Select || !ready_[ 0 ] || !ready_[ 1 ] ) {
		return;
	}
	phase_ = Phase::Countdown;
	countdownT_ = GameConst::startCountdown;
	std::cout << "둘 다 준비, " << GameConst::startCountdown << "초 뒤 시작\n";
}

void GameLogic::sendSelection( std::uint8_t playerNum ) {
	auto selected = Protocol::S2C_CharacterSelected{ };
	selected.playerNum = playerNum;
	selected.character = characters_[ playerNum - 1 ];
	selected.confirmed = ready_[ playerNum - 1 ] ? 1 : 0;
	SessionManager::broadcast( &selected, sizeof( selected ) );
}

void GameLogic::startMatch( ) {
	auto stageDist = std::uniform_int_distribution<int>( 1, GameConst::stageCount );
	stage_ = static_cast<std::uint8_t>( stageDist( rng ) );

	// Match::start가 플레이어 초기화와 Timer::startMatch를 함께 함
	match_.start( characters_[ 0 ], characters_[ 1 ] );
	scores_.fill( 0 );
	phase_ = Phase::Playing;
	started_ = true;
	std::cout << "경기 시작 (스테이지 " << static_cast<int>( stage_ ) << ")\n";
}

void GameLogic::update( ) {
	Timer::update( );

	switch ( phase_ ) {
	case Phase::Countdown:
		countdownT_ -= Timer::fdt( );
		if ( countdownT_ <= 0.f ) {
			startMatch( );
		}
		break;

	case Phase::Playing:
		// 시간이 끝난 틱에는 진행하지 않고 종료, 다시 준비하면 재경기
		if ( Timer::isFinished( ) ) {
			phase_ = Phase::Select;
			ready_.fill( false );
			ended_ = true;
			std::cout << "경기 종료 (" << static_cast<int>( scores_[ 0 ] ) << " : "
				<< static_cast<int>( scores_[ 1 ] ) << ")\n";
			break;
		}
		match_.tick( );
		break;

	default:
		break;
	}
}

void GameLogic::render( ) {
	auto frame = Protocol::S2C_Frame{ };
	frame.frame = Timer::frame( );
	frame.dt = Timer::fdt( );
	frame.fps = Timer::fps( );
	frame.remainTime = Timer::remainTime( );
	frame.finished = Timer::isFinished( ) ? 1 : 0;
	frame.countdown = phase_ == Phase::Countdown
		? static_cast<std::uint8_t>( std::ceil( countdownT_ ) ) : 0;
	SessionManager::broadcast( &frame, sizeof( frame ) );

	// 경기 시작은 첫 상태보다 먼저 보냄
	if ( started_ ) {
		started_ = false;

		auto gameStart = Protocol::S2C_GameStart{ };
		gameStart.stage = stage_;
		gameStart.p1Character = characters_[ 0 ];
		gameStart.p2Character = characters_[ 1 ];
		SessionManager::broadcast( &gameStart, sizeof( gameStart ) );
	}

	if ( phase_ == Phase::Playing ) {
		auto state = Protocol::S2C_GameState{ };
		for ( auto i = 0; i < Protocol::maxPlayers; ++i ) {
			state.players[ i ] = match_.player( static_cast<std::uint8_t>( i + 1 ) ).toState( );
			state.scores[ i ] = scores_[ i ];
		}
		state.remainTime = Timer::remainTime( );
		SessionManager::broadcast( &state, sizeof( state ) );

		auto ball = match_.ball( ).getState( );
		SessionManager::broadcast( &ball, sizeof( ball ) );
	}

	// 종료 후 준비가 풀린 것도 같이 알림
	if ( ended_ ) {
		ended_ = false;

		auto gameEnd = Protocol::S2C_GameEnd{ };
		for ( auto i = 0; i < Protocol::maxPlayers; ++i ) {
			gameEnd.scores[ i ] = scores_[ i ];
		}
		SessionManager::broadcast( &gameEnd, sizeof( gameEnd ) );
		for ( std::uint8_t num = 1; num <= Protocol::maxPlayers; ++num ) {
			sendSelection( num );
		}
	}
}

std::array<JobQueue, Protocol::maxPlayers> GameLogic::playerQueues_;
std::queue<Job> GameLogic::jobs_;
double GameLogic::nextTick_;

Phase GameLogic::phase_ = Phase::Waiting;
float GameLogic::countdownT_ = 0.f;

Match GameLogic::match_;
std::uint8_t GameLogic::stage_ = 1;
bool GameLogic::started_ = false;
bool GameLogic::ended_ = false;

std::array<bool, Protocol::maxPlayers> GameLogic::joined_{ };
std::array<Protocol::Character, Protocol::maxPlayers> GameLogic::characters_{ };
std::array<bool, Protocol::maxPlayers> GameLogic::ready_{ };
std::array<std::uint8_t, Protocol::maxPlayers> GameLogic::scores_{ };
