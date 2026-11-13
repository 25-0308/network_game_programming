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
        void reset(PointFloat pos);
        // 상대 충돌 영역의 중심과 전체 크기를 전달한다. 반환값은 반사 직전의 접근 속도다.
        float onCollisionEntry(PointFloat finalPos, PointFloat scale, float minPushSpeed);
        float onCollision(PointFloat finalPos, PointFloat scale, float minPushSpeed);

        void setPosition(float x, float y);
        void setVelocity(float vx, float vy);
        Protocol::S2C_Ball getState() const;

    private:
        PointFloat pos_;
        PointFloat scale_;
        float velo_x;
        float velo_y;

        float bounceOffBox(PointFloat finalPos, PointFloat scale, float minPushSpeed);
    };

}

#endif
