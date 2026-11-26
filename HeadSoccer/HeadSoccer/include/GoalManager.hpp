#ifndef __goal_manager_hpp
#define __goal_manager_hpp

#include "Object.hpp"
#include "Image.hpp"

// Keeps the score UI and shows the goal screen; goals and kickoff are decided by the server.
class GoalManager : public Object {
public:
	GoalManager( );
	virtual ~GoalManager( );

	void update( ) override;
	void update( int p1Score, int p2Score, int scorer, bool celebrating );
	void render( HDC hdc ) override;

private:
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
