#include "Hole11.h"
#include <cmath>

static const float H11_LEN    = H11_XE - H11_XW;             // 16.0
static const float H11_MID_X  = (H11_XW + H11_XE) * 0.5f;    // 30.0
static const float H11_DEPTH  = H11_ZS - H11_ZN;             //  4.0
static const float H11_TEE_X  = H11_XW + 2.0f;               // 24.0  (near west wall)
static const float H11_CUP_X  = H11_XE - 2.0f;               // 36.0  (near east wall, by bridge)

Hole11::Hole11(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(11, q, b, c, t) {}

void Hole11::render(const mat4& vp) const {
    // Fairway floor
    { mat4 m = glm::translate(mat4(1), {H11_MID_X, 0.f, H11_ZC});
      m = glm::scale(m, {H11_LEN, 1.f, H11_DEPTH});
      draw(*mQuad, m, vp, 0); }

    // Tee box
    { mat4 m = glm::translate(mat4(1), {H11_TEE_X, 0.005f, H11_ZC});
      m = glm::scale(m, {1.5f, 1.f, H11_DEPTH - 0.2f});
      draw(*mQuad, m, vp, 1); }

    // North wall
    drawWall(vp, H11_MID_X, H11_WH * 0.5f, H11_ZN - H11_WTH * 0.5f,
             H11_LEN + H11_WTH * 2.f, H11_WH, H11_WTH);

    // South wall
    drawWall(vp, H11_MID_X, H11_WH * 0.5f, H11_ZS + H11_WTH * 0.5f,
             H11_LEN + H11_WTH * 2.f, H11_WH, H11_WTH);

    // West cap (entry)
    drawWall(vp, H11_XW - H11_WTH * 0.5f, H11_WH * 0.5f, H11_ZC,
             H11_WTH, H11_WH, H11_DEPTH + H11_WTH * 2.f);

    // East cap
    drawWall(vp, H11_XE + H11_WTH * 0.5f, H11_WH * 0.5f, H11_ZC,
             H11_WTH, H11_WH, H11_DEPTH + H11_WTH * 2.f);

    // Cup
    drawCup(vp, H11_CUP_X, H11_ZC);
}

void Hole11::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f;

    if (pos.z - BR < H11_ZN) { pos.z = H11_ZN + BR; if (vel.z < 0) vel.z =  fabsf(vel.z) * r; }
    if (pos.z + BR > H11_ZS) { pos.z = H11_ZS - BR; if (vel.z > 0) vel.z = -fabsf(vel.z) * r; }
    if (pos.x - BR < H11_XW) { pos.x = H11_XW + BR; if (vel.x < 0) vel.x =  fabsf(vel.x) * r; }
    if (pos.x + BR > H11_XE) { pos.x = H11_XE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r; }
}

bool Hole11::nearCup(const vec3& pos) const {
    float dx = pos.x - H11_CUP_X;
    float dz = pos.z - H11_ZC;
    return sqrtf(dx * dx + dz * dz) < 0.35f;
}

vec3 Hole11::getTeePos() const {
    return { H11_TEE_X, 0.08f, H11_ZC };
}
