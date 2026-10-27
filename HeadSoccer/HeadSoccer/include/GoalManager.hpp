#ifndef __goal_manager_hpp
#define __goal_manager_hpp

#include "Object.hpp"
#include "Image.hpp"

// Detects goals, keeps the score UI, shows the goal screen and resets to kickoff.
class GoalManager : public Object {
public:
	GoalManager( );
	virtual ~GoalManager( );

	void update( ) override;
	void render( HDC hdc ) override;

private:
	void scoreGoal( int scorer );
	void kickoff( );
	void renderScore( HDC hdc, int score, PointFloat pos );
	void renderGoalScreen( HDC hdc );

	std::vector<Image*> digits_;
	HFONT bigFont_;
	HFONT smallFont_;
	HDC bandDC_;
	HBITMAP bandBmp_;
	HBITMAP oldBandBmp_;
	int lastScorer_;
	float celebrateT_;
	bool celebrating_;
};

#endif	// __goal_manager_hpp
