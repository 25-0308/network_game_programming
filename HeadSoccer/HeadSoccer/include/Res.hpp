#ifndef __res_hpp
#define __res_hpp

#include "common.hpp"

class Resource {	
public:
	Resource( ) : key_( ) {}
	Resource( std::string_view key ) : key_( key ) {}

	virtual ~Resource( ) = 0 {}

private:
	std::string key_;

};

#endif	// __res_hpp