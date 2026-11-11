#ifndef __protocol_hpp
#define __protocol_hpp

#include <cstdint>

// 서버와 클라이언트가 같이 쓰는 패킷 정의
// 양쪽 모두 Windows(리틀 엔디언)라 구조체를 그대로 보냄 (1바이트 정렬)

namespace Protocol {

	constexpr std::uint16_t serverPort = 9000;
	constexpr int maxPlayers = 2;

	enum class PacketType : std::uint8_t {
		// 클라 -> 서버
		C2S_SelectCharacter = 1,
		C2S_Input,
		C2S_Rematch,

		// 서버 -> 클라
		S2C_Login = 101,
		S2C_ChangeScene,
		S2C_CharacterSelected,
		S2C_GameStart,
		S2C_GameState,
		S2C_Ball,
		S2C_Goal,
		S2C_GameEnd,
		S2C_EnemyLeft,
	};

	// 이름순 (ImageStorage의 map 순서와 같음)
	enum class Character : std::uint8_t {
		America,
		Argentina,
		France,
		Korea,
	};

	// C2S_Input::keys 비트
	enum InputKey : std::uint8_t {
		KeyLeft = 1 << 0,
		KeyRight = 1 << 1,
		KeyJump = 1 << 2,
		KeyKick = 1 << 3,
	};

	enum class PlayerAnim : std::uint8_t {
		Idle,
		RunFront,
		RunBack,
		Kick,
	};

#pragma pack( push, 1 )

	struct PacketHeader {
		std::uint16_t size;		// 헤더 포함 전체 크기
		PacketType type;
	};

	// ---------------- 클라 -> 서버 ----------------

	// 캐릭터 선택 중 및 확정
	struct C2S_SelectCharacter {
		PacketHeader header{ sizeof( C2S_SelectCharacter ), PacketType::C2S_SelectCharacter };
		Character character;
		std::uint8_t confirmed;	// 0: 선택 중, 1: 확정
	};

	// 누르고 있는 키, 바뀔 때만 보냄
	struct C2S_Input {
		PacketHeader header{ sizeof( C2S_Input ), PacketType::C2S_Input };
		std::uint8_t keys;		// InputKey 비트
	};

	// 경기 종료 후 다시 시작
	struct C2S_Rematch {
		PacketHeader header{ sizeof( C2S_Rematch ), PacketType::C2S_Rematch };
	};

	// ---------------- 서버 -> 클라 ----------------

	// 접속한 플레이어 번호
	struct S2C_Login {
		PacketHeader header{ sizeof( S2C_Login ), PacketType::S2C_Login };
		std::uint8_t playerNum;	// 1(왼쪽) 또는 2(오른쪽)
	};

	// 캐릭터 선택 화면으로 전환
	struct S2C_ChangeScene {
		PacketHeader header{ sizeof( S2C_ChangeScene ), PacketType::S2C_ChangeScene };
	};

	// 상대 캐릭터 선택 확인용
	struct S2C_CharacterSelected {
		PacketHeader header{ sizeof( S2C_CharacterSelected ), PacketType::S2C_CharacterSelected };
		std::uint8_t playerNum;
		Character character;
		std::uint8_t confirmed;
	};

	// 경기장과 두 캐릭터를 알리고 경기 시작
	struct S2C_GameStart {
		PacketHeader header{ sizeof( S2C_GameStart ), PacketType::S2C_GameStart };
		std::uint8_t stage;		// 1 ~ 4, 서버가 랜덤으로 정함
		Character p1Character;
		Character p2Character;
	};

	struct PlayerState {
		float x;
		float y;
		std::uint8_t hp;		// 0 ~ 100
		PlayerAnim anim;
		std::uint8_t stunned;	// 1이면 기절
	};

	// 두 플레이어 상태, 남은 시간, 점수
	struct S2C_GameState {
		PacketHeader header{ sizeof( S2C_GameState ), PacketType::S2C_GameState };
		PlayerState players[ maxPlayers ];	// [0] = 1P, [1] = 2P
		std::uint16_t remainTime;	// 초
		std::uint8_t scores[ maxPlayers ];
	};

	// 공 위치와 속도 (속력, 방향)
	struct S2C_Ball {
		PacketHeader header{ sizeof( S2C_Ball ), PacketType::S2C_Ball };
		float x;
		float y;
		float vx;
		float vy;
	};

	// 골 발생 후 종합 점수
	struct S2C_Goal {
		PacketHeader header{ sizeof( S2C_Goal ), PacketType::S2C_Goal };
		std::uint8_t scores[ maxPlayers ];
	};

	// 게임 종료 후 최종 점수
	struct S2C_GameEnd {
		PacketHeader header{ sizeof( S2C_GameEnd ), PacketType::S2C_GameEnd };
		std::uint8_t scores[ maxPlayers ];
	};

	// 게임 중 상대 접속 종료
	struct S2C_EnemyLeft {
		PacketHeader header{ sizeof( S2C_EnemyLeft ), PacketType::S2C_EnemyLeft };
	};

#pragma pack( pop )

	static_assert( sizeof( PacketHeader ) == 3 );
	static_assert( sizeof( C2S_SelectCharacter ) == 5 );
	static_assert( sizeof( C2S_Input ) == 4 );
	static_assert( sizeof( C2S_Rematch ) == 3 );
	static_assert( sizeof( S2C_Login ) == 4 );
	static_assert( sizeof( S2C_ChangeScene ) == 3 );
	static_assert( sizeof( S2C_CharacterSelected ) == 6 );
	static_assert( sizeof( S2C_GameStart ) == 6 );
	static_assert( sizeof( PlayerState ) == 11 );
	static_assert( sizeof( S2C_GameState ) == 29 );
	static_assert( sizeof( S2C_Ball ) == 19 );
	static_assert( sizeof( S2C_Goal ) == 5 );
	static_assert( sizeof( S2C_GameEnd ) == 5 );
	static_assert( sizeof( S2C_EnemyLeft ) == 3 );

}	// namespace Protocol

#endif	// __protocol_hpp
