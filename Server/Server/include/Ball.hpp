#ifndef HEADSOCCER_SERVER_BALL_HPP
#define HEADSOCCER_SERVER_BALL_HPP

#include "../../../Common/Protocol.hpp"
#include "../../../Common/GameConstants.hpp"
#include "../../../Common/PointFloat.hpp"

namespace ServerGame {

    class Ball {
    public:
        Ball(float x = GameConst::ballStartX, float y = GameConst::ballStartY, float radius = GameConst::ballRadius);

        void update(float dt);
        void kickleft();
        void kickright();
        void reset(PointFloat pos);
        // 상대 충돌 영역의 중심과 전체 크기를 전달한다. 닿지 않았으면 아무것도 바꾸지 않는다.
        void onCollision(PointFloat finalPos, PointFloat scale, float minPushSpeed);

        Protocol::S2C_Ball getState() const;

    private:
        PointFloat pos_;
        PointFloat scale_;
        PointFloat velo_;

        void bounceOffBox(PointFloat finalPos, PointFloat scale, float minPushSpeed);
    };

}

#endif
