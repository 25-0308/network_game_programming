#include "../include/Ball.hpp"

#include <algorithm>
#include <cmath>

namespace ServerGame {

    Ball::Ball(float x, float y, float radius)
        : pos_{x, y}, scale_{radius, radius}, velo_{GameConst::ballStartVx, GameConst::ballStartVy} {}

    void Ball::update(float dt) {
        if (!std::isfinite(dt) || dt <= 0.f) {
            return;
        }

        velo_.x *= std::pow(GameConst::ballFrictionPerSec, dt);
        velo_.y += GameConst::ballGravity * dt;
        pos_.x += velo_.x * dt;
        pos_.y += velo_.y * dt;

        // 벽: 공 중심에서 반지름만큼 나간 곳이 경기장 경계에 닿으면 튕김
        if (pos_.x - scale_.x <= GameConst::fieldLeft) {
            velo_.x = -velo_.x;
            pos_.x = GameConst::fieldLeft + scale_.x;
        }
        else if (pos_.x + scale_.x >= GameConst::fieldRight) {
            velo_.x = -velo_.x;
            pos_.x = GameConst::fieldRight - scale_.x;
        }

        if (pos_.y - scale_.y <= GameConst::ballCeilingY) {
            velo_.y = -velo_.y;
            pos_.y = GameConst::ballCeilingResetY + scale_.y;
        }
        else if (pos_.y + scale_.y >= GameConst::ballGroundY) {
            velo_.y = -velo_.y * GameConst::ballGroundRestitution;
            pos_.y = GameConst::ballGroundY - scale_.y;
        }
    }

    void Ball::kickleft() {
        velo_.x -= GameConst::ballKickVx;
        velo_.y += GameConst::ballKickVy;
    }

    void Ball::kickright() {
        velo_.x += GameConst::ballKickVx;
        velo_.y += GameConst::ballKickVy;
    }

    void Ball::reset(PointFloat pos) {
        pos_ = pos;
        velo_ = PointFloat(GameConst::ballStartVx, GameConst::ballStartVy);
    }

    void Ball::onCollision(PointFloat finalPos, PointFloat scale, float minPushSpeed) {
        bounceOffBox(finalPos, scale, minPushSpeed);
    }

    void Ball::bounceOffBox(PointFloat finalPos, PointFloat scale, float minPushSpeed) {
        const float radius = scale_.x;
        const float restitution = GameConst::ballBoxRestitution;

        auto boxPos = finalPos;
        auto half = scale / 2.f;
        float left = boxPos.x - half.x;
        float right = boxPos.x + half.x;
        float top = boxPos.y - half.y;
        float bottom = boxPos.y + half.y;

        auto closest = PointFloat(std::clamp(pos_.x, left, right), std::clamp(pos_.y, top, bottom));
        auto normal = pos_ - closest;
        float dist = std::hypot(normal.x, normal.y);

        if (dist > 0.f) {
            // 사각형끼리 겹쳐도 실제 공의 원은 꼭짓점에 닿지 않을 수 있다.
            if (dist >= radius) {
                return;
            }
            normal = normal / dist;
            pos_ += normal * (radius - dist);
        }
        else {
            // 공 중심이 상자 안에 있으면 가장 가까운 면으로 밀어낸다.
            float toLeft = pos_.x - left;
            float toRight = right - pos_.x;
            float toTop = pos_.y - top;
            float toBottom = bottom - pos_.y;
            float nearest = std::min({toLeft, toRight, toTop, toBottom});

            if (nearest == toTop) {
                normal = PointFloat(0.f, -1.f);
                pos_.y = top - radius;
            }
            else if (nearest == toLeft) {
                normal = PointFloat(-1.f, 0.f);
                pos_.x = left - radius;
            }
            else if (nearest == toRight) {
                normal = PointFloat(1.f, 0.f);
                pos_.x = right + radius;
            }
            else {
                normal = PointFloat(0.f, 1.f);
                pos_.y = bottom + radius;
            }
        }

        float vn = velo_.x * normal.x + velo_.y * normal.y;
        if (vn < 0.f) {
            velo_ -= normal * (vn * (1.f + restitution));
            vn = -vn * restitution;
        }
        if (vn < minPushSpeed) {
            velo_ += normal * (minPushSpeed - vn);
        }
    }

    Protocol::S2C_Ball Ball::getState() const {
        auto packet = Protocol::S2C_Ball{};
        packet.x = pos_.x;
        packet.y = pos_.y;
        packet.vx = velo_.x;
        packet.vy = velo_.y;
        return packet;
    }

}
