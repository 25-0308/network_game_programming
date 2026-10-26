#ifndef __res_hpp
#define __res_hpp

#include "common.hpp"

class Resource {	
public:
	Resource( ) : key_( ) {}
	Resource( std::string_view key ) : key_( key ) {}

	virtual ~Resource( ) = 0 {}

	void setKey( const std::string& key ) { key_ = key; }
	const std::string& getKey( ) const { return key_; }

private:
	std::string key_;

};

#endif	// __res_hpp