#include "../include/Ball.hpp"

#include <algorithm>
#include <cmath>

namespace ServerGame {

    Ball::Ball(float x, float y, float radius)
        : pos_{x, y}, scale_{radius, radius}, velo_x{1.f}, velo_y{-100.f} {}

    float Ball::update(float dt) {
        if (!std::isfinite(dt) || dt <= 0.f) {
            return 0.f;
        }

        velo_x *= std::pow(0.7f, dt);
        velo_y += 1000.f * dt;
        pos_.x += velo_x * dt;
        pos_.y += velo_y * dt;

        constexpr float fieldWidth = 1280.f;
        constexpr float fieldHeight = 650.5f;
        float impactSpeed = 0.f;

        if (pos_.x - scale_.x <= 0.f) {
            impactSpeed = std::max(impactSpeed, -velo_x);
            velo_x = -velo_x;
            pos_.x = scale_.x;
        }
        else if (pos_.x + scale_.x >= fieldWidth) {
            impactSpeed = std::max(impactSpeed, velo_x);
            velo_x = -velo_x;
            pos_.x = fieldWidth - scale_.x;
        }

        if (pos_.y - scale_.y <= 10.f) {
            impactSpeed = std::max(impactSpeed, -velo_y);
            velo_y = -velo_y;
            pos_.y = 20.f + scale_.y;
        }
        else if (pos_.y + scale_.y >= fieldHeight) {
            impactSpeed = std::max(impactSpeed, velo_y);
            velo_y = -velo_y * 0.8f;
            pos_.y = fieldHeight - scale_.y;
        }

        return impactSpeed;
    }

    void Ball::kickleft() {
        velo_x -= 2000.f;
        velo_y -= 1000.f;
    }

    void Ball::kickright() {
        velo_x += 2000.f;
        velo_y -= 1000.f;
    }

    void Ball::reset(PointFloat pos) {
        pos_ = pos;
        velo_x = 1.f;
        velo_y = -100.f;
    }

    float Ball::onCollisionEntry(PointFloat finalPos, PointFloat scale, float minPushSpeed) {
        return onCollision(finalPos, scale, minPushSpeed);
    }

    float Ball::onCollision(PointFloat finalPos, PointFloat scale, float minPushSpeed) {
        return bounceOffBox(finalPos, scale, minPushSpeed);
    }

    float Ball::bounceOffBox(PointFloat finalPos, PointFloat scale, float minPushSpeed) {
        auto velocity = PointFloat(velo_x, velo_y);
        const float radius = scale_.x;
        const float restitution = 0.8f;

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
                return 0.f;
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

        float impactSpeed = 0.f;
        float vn = velocity.x * normal.x + velocity.y * normal.y;
        if (vn < 0.f) {
            impactSpeed = -vn;
            velocity -= normal * (vn * (1.f + restitution));
            vn = -vn * restitution;
        }
        if (vn < minPushSpeed) {
            velocity += normal * (minPushSpeed - vn);
        }

        velo_x = velocity.x;
        velo_y = velocity.y;
        return impactSpeed;
    }

    void Ball::setPosition(float x, float y) {
        pos_.x = x;
        pos_.y = y;
    }

    void Ball::setVelocity(float vx, float vy) {
        velo_x = vx;
        velo_y = vy;
    }

    Protocol::S2C_Ball Ball::getState() const {
        auto packet = Protocol::S2C_Ball{};
        packet.x = pos_.x;
        packet.y = pos_.y;
        packet.vx = velo_x;
        packet.vy = velo_y;
        return packet;
    }

}
