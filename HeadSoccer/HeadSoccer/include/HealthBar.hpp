#ifndef __health_bar_hpp
#define __health_bar_hpp

#include "Object.hpp"

class Player;

// HP bar in a top corner of the screen, plus stars circling the player's head while knocked out.
class HealthBar : public Object {
public:
	HealthBar( const Player* player, bool rightSide );
	virtual ~HealthBar( );

	HealthBar( const HealthBar& ) = delete;

	void update( ) override;
	void render( HDC hdc ) override;

private:
	void renderStunStars( HDC hdc );

	const Player* player_;
	bool rightSide_;
	float starAngle_;

	HBRUSH bgBrush_;
	HBRUSH highBrush_;
	HBRUSH midBrush_;
	HBRUSH lowBrush_;
	HBRUSH starBrush_;
	HPEN starPen_;
};

#endif	// __health_bar_hpp
