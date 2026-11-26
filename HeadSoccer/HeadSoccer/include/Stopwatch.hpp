#ifndef __stopwatch_hpp
#define __stopwatch_hpp

#include "Object.hpp"
#include "Image.hpp"

class Stopwatch : public Object {
public:
	// 남은 시간은 서버가 S2C_Frame으로 보낸 Timer::remainTime을 그대로 그림
	Stopwatch( PointFloat pos );
	virtual ~Stopwatch( ) {}

	void render( HDC hdc ) override;

private:
	std::vector<Image*> digits;
};

#endif  // __stopwatch_hpp
