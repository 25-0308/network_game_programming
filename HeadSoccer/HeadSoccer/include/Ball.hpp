#ifndef __ball_hpp
#define __ball_hpp

#include "Object.hpp"
#include "PointFloat.hpp"
#include "ResourceHandler.hpp"
#include "Image.hpp"
#include "../../../Common/Protocol.hpp"


class Ball : public Object {
public:
    Ball(std::string name);

    void update() override;
    void update( const Protocol::S2C_Ball& state );
    void render(HDC hdc)override;
    void applyState( const Protocol::S2C_Ball& state );
private:
    Image* image;
    PointFloat velo_;
    bool receivedState_ = false;
    float impactSoundCooldown_ = 0.f;

    void playImpactSound( float impactSpeed );
};

#endif	// __ball_hpp
