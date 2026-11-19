#include "../include/JobQueue.hpp"

void JobQueue::push( const Job& job ) {
	jobs_.push( job );
}

bool JobQueue::pop( Job& job ) {
	if ( jobs_.empty( ) ) {
		return false;
	}
	job = jobs_.front( );
	jobs_.pop( );
	return true;
}
