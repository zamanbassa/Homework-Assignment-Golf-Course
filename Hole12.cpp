#include "Hole12.h"
#include <cmath>

static const float H12_LEN    = H12_ZS - H12_ZN;             // 10.0
static const float H12_MID_Z  = (H12_ZN + H12_ZS) * 0.5f;    // -18.0
static const float H12_TEE_Z  = H12_ZN + 2.0f;               // -21.0  (near north)
static const float H12_CUP_Z  = H12_ZS - 2.0f;               // -15.0  (near south)

Hole12::Hole12(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(12, q, b, c, t) {}

void Hole12::render(const mat4& vp) const {
    // Bridge floor (wood, slightly elevated above river)
    { mat4 m = glm::translate(mat4(1), {H12_CX, H12_FLOOR_Y, H12_MID_Z});
      m = glm::scale(m, {H12_FW, 1.f, H12_LEN});
      draw(*mQuad, m, vp, 6); }  // surface 6 = wood

    // Tee box
    { mat4 m = glm::translate(mat4(1), {H12_CX, H12_FLOOR_Y + 0.005f, H12_TEE_Z});
      m = glm::scale(m, {H12_FW - 0.2f, 1.f, 1.2f});
      draw(*mQuad, m, vp, 1); }

    // East railing (full length) — N/S ends are OPEN (bridge entrance / exit)
    drawWall(vp, H12_XE + H12_WTH * 0.5f, H12_WH * 0.5f + H12_FLOOR_Y, H12_MID_Z,
             H12_WTH, H12_WH, H12_LEN);

    // West railing
    drawWall(vp, H12_XW - H12_WTH * 0.5f, H12_WH * 0.5f + H12_FLOOR_Y, H12_MID_Z,
             H12_WTH, H12_WH, H12_LEN);

    // Bridge support posts at corners + midpoints (wooden cylinders)
    auto post = [&](float px, float pz) {
        mat4 m = glm::translate(mat4(1), {px, 0.f, pz});
        m = glm::scale(m, {H12_POST_R * 2.f, H12_WH * 1.8f, H12_POST_R * 2.f});
        draw(*mCylinder, m, vp, 6);  // wood
    };
    post(H12_XW, H12_ZN); post(H12_XE, H12_ZN);
    post(H12_XW, H12_ZS); post(H12_XE, H12_ZS);
    post(H12_XW, H12_MID_Z); post(H12_XE, H12_MID_Z);

    // Cup
    drawCup(vp, H12_CX, H12_CUP_Z);
}

void Hole12::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f;

    // Only side rails — N/S ends open so ball can enter and exit the bridge
    if (pos.x - BR < H12_XW) { pos.x = H12_XW + BR; if (vel.x < 0) vel.x =  fabsf(vel.x) * r; }
    if (pos.x + BR > H12_XE) { pos.x = H12_XE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r; }
}

bool Hole12::nearCup(const vec3& pos) const {
    float dx = pos.x - H12_CX;
    float dz = pos.z - H12_CUP_Z;
    return sqrtf(dx * dx + dz * dz) < 0.35f;
}

vec3 Hole12::getTeePos() const {
    return { H12_CX, H12_FLOOR_Y + 0.08f, H12_TEE_Z };
}
