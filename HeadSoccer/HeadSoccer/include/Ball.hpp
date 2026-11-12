#ifndef __ball_hpp
#define __ball_hpp

#include "Object.hpp"
#include "PointFloat.hpp"
#include "ResourceHandler.hpp"
#include "Image.hpp"
#include "../../../Server/Server/include/Ball.hpp"


class Ball : public Object {
public:
    Ball(std::string name);

    void update() override;
    void render(HDC hdc)override;
    void applyState( const Protocol::S2C_Ball& state );
    void kickleft();
    void kickright();
    void reset( PointFloat pos );
    void onCollisionEntry( Object* other ) override;
    void onCollision( Object* other ) override;
private:
    Image* image;
    // 로컬에서도 같은 계산식
    ServerGame::Ball simulation_;
    float impactSoundCooldown_ = 0.f;

    void playImpactSound( float impactSpeed );

    void bounceOffBox( const Collider* box, float minPushSpeed );
};

#endif	// __ball_hpp
