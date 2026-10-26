#include "../include/Ball.hpp"
#include "../include/Timer.hpp"
#include "../include/Camera.hpp"
#include "../include/InputHandler.hpp"

void Ball::kickleft() {
    velo_.x -= 2000.0f;
    velo_.y -= 1000.0f;
}

void Ball::kickright( ) {
    velo_.x += 2000.0f;
    velo_.y -= 1000.0f;
}

void Ball::render(HDC hdc) {
    auto relativePos = pos_ - Camera::getPos();
    if (image) {
        image->draw(hdc, relativePos);
    }
    componentRender(hdc);
}

void Ball::update() {
	if ( InputHandler::getKeyState( Key::R ) == KeyState::Tap ) {
        kickright( );
	}
	if ( InputHandler::getKeyState( Key::Q ) == KeyState::Tap ) {
		kickleft( );
	}

    float dt = Timer::fdt();
    static float timer = 0;
    timer += dt;
    if (timer >= 1.0f) { // 좌우 속도 감소
        velo_.x = velo_.x * 0.7;
        timer = 0.0f;
    }
    velo_.y += 1000.0f * dt; // 중력
    pos_ += velo_ * dt;

    float fieldWidth = 1280.0f;
    float fieldHeight = 615.5f;

    if (pos_.x - scale_.x <= 0.0f) { // 왼벽
        velo_.x = -velo_.x;
        pos_.x = 0.0f + scale_.x;
    }
    else if (pos_.x + scale_.x >= fieldWidth) { // 오른벽
        velo_.x = -velo_.x;
        pos_.x = fieldWidth - scale_.x;
    }

    if (pos_.y - scale_.y <= 10.0f) { // 천장 충돌
        velo_.y = -velo_.y;
        pos_.y = 20.0f + scale_.y;
    }
    else if (pos_.y + scale_.y >= fieldHeight) { //지면 충돌
        velo_.y = -velo_.y * 0.8f;
        pos_.y = fieldHeight - scale_.y;
    }

    componentUpdate( );
}