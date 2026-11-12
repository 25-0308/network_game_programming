#ifndef HEADSOCCER_SERVER_BALL_HPP
#define HEADSOCCER_SERVER_BALL_HPP

#include "../../../Common/Protocol.hpp"
#include "../../../Common/PointFloat.hpp"

namespace ServerGame {

    class Ball {
    public:
        Ball(float x = 640.f, float y = 360.f, float radius = 35.f);

        float update(float dt);
        void kickleft();
        void kickright();

        void setPosition(float x, float y);
        void setVelocity(float vx, float vy);
        Protocol::S2C_Ball getState() const;

    private:
        PointFloat pos_;
        PointFloat scale_;
        float velo_x;
        float velo_y;
    };

}

#endif
