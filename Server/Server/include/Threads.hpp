#ifndef __threads_hpp
#define __threads_hpp

#include "common.hpp"

// ================ 서버 스레드 구조 ================
//
// 메인 스레드 (1개)
//   접속 받기(블로킹 accept) -> 자리 예약 -> S2C_Login 전송 -> 수신 스레드 생성(멈춤)
//   -> 자리 등록 -> 그 플레이어 큐에 PlayerJoin -> 수신 스레드 실행
//   서버 종료 시 게임 로직 스레드와 모든 수신 스레드가 끝나길 기다리고 정리
//
// 플레이어 수신 스레드 (접속마다 1개, 최대 2개)
//   자기 소켓만 읽어서(블로킹 recv) 자기 플레이어 큐에만 넣음
//   끊김, 멈추라는 요청, 큐 넘침이면 PlayerLeave를 마지막으로 넣고 끝남
//
// 게임 로직 스레드 (1개)
//   매 틱: 1P 큐, 2P 큐를 번갈아 전체 큐로 가져옴 -> 순서대로 실행 -> 경기 진행 -> 전송
//   패킷 처리, 경기 상태, Timer, S2C_Login 이후의 모든 전송, 자리 정리는 여기서만 함
//
// ================ 데이터와 락 ================
//
// 플레이어 큐 x2 (JobQueue)   메인, 수신, 게임 로직이 씀     -> 큐마다 락 하나
// 세션 정보 (SessionManager)  메인, 수신, 게임 로직이 씀     -> 세션 락 하나
// 전체 큐, 경기 상태, Timer   게임 로직만 씀                 -> 락 없음
// 리슨 소켓, 서버 실행 상태   메인, 종료 처리가 씀           -> atomic
//
// ================ 지켜야 할 규칙 ================
//
// 1. 락은 한 번에 하나만 잡음 (락을 잡은 채로 다른 락을 잡지 않음 -> 교착 없음)
// 2. 락 안에서는 send, recv, 스레드 대기처럼 막힐 수 있는 일을 하지 않음
// 3. 큐 락 안에서는 넣기나 통째로 맞바꾸기만 함
// 4. 자리는 PlayerLeave를 처리할 때만 비우고, 그 수신 스레드가 끝난 뒤에 소켓을 닫음
//    (닫은 소켓 번호를 새 접속이 다시 받아도 예전 스레드가 읽지 않음)
// 5. 한 플레이어의 작업은 Join -> 패킷들 -> Leave 순서로만 큐에 들어감

// 서버가 실행 중인지 (false가 되면 모든 스레드가 끝남)
bool isRunning( );

// [종료 처리] 서버 종료 요청
void requestShutdown( );

// 플레이어 패킷 하나를 받아 그 플레이어 큐에 넣음
// 끊김, 멈추라는 요청, 큐 넘침이면 PlayerLeave를 넣고 false (스레드의 마지막 작업)
bool receivePlayerPacket( std::uint8_t playerNum, SOCKET sock );

// 플레이어 1 수신 스레드 (접속할 때마다 새로 만들고, 끊기면 끝남)
DWORD WINAPI player_1_thread( LPVOID arg );

// 플레이어 2 수신 스레드
DWORD WINAPI player_2_thread( LPVOID arg );

// 게임 로직 스레드 (서버 시작 시 하나, isRunning이 false가 되면 끝남)
DWORD WINAPI game_logic_thread( LPVOID arg );

#endif	// __threads_hpp
