#ifndef __job_queue_hpp
#define __job_queue_hpp

#include "Job.hpp"

#include <queue>

// 넣은 순서대로 꺼내는 작업큐
// 지금은 메인 스레드 하나만 접근하므로 락 없음 (멀티스레드 전환 시 추가)
class JobQueue {
public:
	void push( const Job& job );

	// 맨 앞 작업을 꺼냄, 비었으면 false
	bool pop( Job& job );

	bool empty( ) const {
		return jobs_.empty( );
	}

private:
	std::queue<Job> jobs_;
};

#endif	// __job_queue_hpp
