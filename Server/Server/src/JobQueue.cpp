#include "../include/JobQueue.hpp"

JobQueue::JobQueue( ) {
	InitializeCriticalSection( &lock_ );
}

JobQueue::~JobQueue( ) {
	DeleteCriticalSection( &lock_ );
}

void JobQueue::push( const Job& job ) {
	EnterCriticalSection( &lock_ );
	jobs_.push( job );
	LeaveCriticalSection( &lock_ );
}

bool JobQueue::tryPush( const Job& job, std::size_t limit ) {
	EnterCriticalSection( &lock_ );
	bool pushed = jobs_.size( ) < limit;
	if ( pushed ) {
		jobs_.push( job );
	}
	LeaveCriticalSection( &lock_ );
	return pushed;
}

void JobQueue::takeAll( std::queue<Job>& out ) {
	auto taken = std::queue<Job>{ };

	EnterCriticalSection( &lock_ );
	std::swap( taken, jobs_ );
	LeaveCriticalSection( &lock_ );

	// 옮겨 담기는 락 밖에서
	while ( !taken.empty( ) ) {
		out.push( taken.front( ) );
		taken.pop( );
	}
}
