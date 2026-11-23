#ifndef __job_queue_hpp
#define __job_queue_hpp

#include "Job.hpp"

#include <queue>

// 플레이어별 작업큐 (플레이어 스레드가 넣고, 게임 로직 스레드가 가져감)
// 두 스레드가 같이 쓰므로 넣고 가져갈 때 락을 잡음
class JobQueue {
public:
	JobQueue( );
	~JobQueue( );

	JobQueue( const JobQueue& ) = delete;
	JobQueue& operator=( const JobQueue& ) = delete;

	void push( const Job& job );

	// 쌓인 개수가 limit보다 적을 때만 넣음, 꽉 찼으면 false
	bool tryPush( const Job& job, std::size_t limit );

	// 쌓인 작업을 out으로 통째로 옮기고 비움 (락은 맞바꾸는 동안만 잡음)
	void takeAll( std::queue<Job>& out );

private:
	CRITICAL_SECTION lock_;
	std::queue<Job> jobs_;
};

#endif	// __job_queue_hpp
