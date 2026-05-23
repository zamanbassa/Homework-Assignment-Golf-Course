#include "Camera.h"

Camera::Camera()
    : pos({-33, 25, 55}), yaw(0), pitch(-0.50f), fov(60.0f), spd(18.0f), ortho(false) {}

vec3 Camera::fwd() const
{
    return {sinf(yaw) * cosf(pitch), sinf(pitch), -cosf(yaw) * cosf(pitch)};
}

mat4 Camera::view() const
{
    return glm::lookAt(pos, pos + fwd(), {0, 1, 0});
}

mat4 Camera::proj(float a) const
{
    if (ortho)
    {
        float h = 18, w = h * a;
        return glm::ortho(-w, w, -h, h, .1f, 400.f);
    }
    return glm::perspective(glm::radians(fov), a, .1f, 400.f);
}

void Camera::setPos(vec3 p) {
    this->pos = p;
}

void Camera::setYaw(float y) {
    this->yaw = y;
}

void Camera::setPitch(float p) {
    this->pitch = p;
}

void Camera::setFOV(float f) {
    this->fov = f;
}

void Camera::setSpd(float s) {
    this->spd = s;
}

void Camera::setOrtho(bool o) {
    this->ortho = o;
}

vec3 Camera::getPos() const {
    return pos;
}

float Camera::getYaw() const {
    return yaw;
}

float Camera::getPitch() const {
    return pitch;
}

float Camera::getFOV() const {
    return fov;
}

float Camera::getSpd() const {
    return spd;
}

bool Camera::getOrtho() const {
    return ortho;
}