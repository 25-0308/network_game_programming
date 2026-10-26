#ifndef __stopwatch_hpp
#define __stopwatch_hpp

#include "Object.hpp"
#include "Image.hpp"

class Stopwatch : public Object {
public:
	Stopwatch( PointFloat pos, std::uint16_t time );
	virtual ~Stopwatch( ) {}

	void update( ) override;
	void render( HDC hdc ) override;

	void onCollision( Object* other ) override {}
	void onCollisionEntry( Object* other ) override {}
	void onCollisionExit( Object* other ) override {}

private:
	std::vector<Image*> digits;
	std::uint16_t time_;
	float accT_;
};

#endif  // __stopwatch_hpp