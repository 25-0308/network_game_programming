#ifndef __event_handler_hpp
#define __event_handler_hpp

#include "common.hpp"
#include "Object.hpp"

struct Event {
	EventType type;
	DWORD_PTR wParam;	// object type, scene type
	DWORD_PTR lParam;	// object pointer
};

class EventHandler {
public:
	static void update( );
	static void execute( const Event& event );

	static void addEvent( const Event& event ) {
		events_.push_back( event );
	}

private:
	static std::vector<Event> events_;
	static std::vector<Object*> deadObjects_;
};

#endif	// __event_handler_hpp