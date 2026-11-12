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
