#ifndef __ball_hpp
#define __ball_hpp

#include "Object.hpp"
#include "PointFloat.hpp"
#include "ResourceHandler.hpp"
#include "Image.hpp"


class Ball : public Object {
public:
    Ball(std::string name) {
        image = ResourceHandler::loadImage(name, "assets/ball.png", true);
        velo_.x = 1.0f;
        velo_.y = -100.0f;
        createCollider( );
        getCollider( )->setScale( PointFloat( static_cast<float>( image->width( ) ), static_cast<float>( image->height( ) ) ) );
        // update() keeps the ball inside the field using scale_ as its radius
        setScale( PointFloat( image->width( ) / 2.f, image->height( ) / 2.f ) );
    }
    PointFloat velo_; // ¼Óµµ 

    void update() override;
    void render(HDC hdc)override;
    void kickleft();
    void kickright();
    void reset( PointFloat pos );
    void onCollisionEntry( Object* other ) override;
    void onCollision( Object* other ) override;
private:
    Image* image;

    void bounceOffBox( const Collider* box, float minPushSpeed );
};

#endif	// __ball_hpp
