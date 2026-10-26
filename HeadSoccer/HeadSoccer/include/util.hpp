#ifndef __util_hpp
#define __util_hpp

#include <cstdint>

// Convert enum to uint8_t
template<class Ty>
constexpr std::uint8_t etoi( Ty e ) {
	return static_cast<std::uint8_t>( e );
}

#endif	// __util_hpp