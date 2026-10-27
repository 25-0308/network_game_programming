#ifndef __health_bar_hpp
#define __health_bar_hpp

#include "Object.hpp"

class Player;

// HP bar in a top corner of the screen, plus a STUN label over the player while knocked out.
class HealthBar : public Object {
public:
	HealthBar( const Player* player, bool rightSide );
	virtual ~HealthBar( );

	HealthBar( const HealthBar& ) = delete;

	void render( HDC hdc ) override;

private:
	const Player* player_;
	bool rightSide_;

	HFONT font_;
	HBRUSH bgBrush_;
	HBRUSH highBrush_;
	HBRUSH midBrush_;
	HBRUSH lowBrush_;
};

#endif	// __health_bar_hpp
