#include "Hole11.h"
#include <cmath>

static const float H11_LEN   = H11_XE - H11_XW;              // 20.0
static const float H11_MID_X = (H11_XW + H11_XE) * 0.5f;    // 24.0
static const float H11_DEPTH = H11_ZS - H11_ZN;              //  4.0
static const float H11_TEE_X = H11_XW + 2.0f;                // 16.0
static const float H11_CUP_X = H11_XE - 2.0f;                // 32.0

Hole11::Hole11(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(11, q, b, c, t) {}

void Hole11::render(const mat4& vp) const
{
    // ── Fairway (grass) ───────────────────────────────────────────────────────
    { mat4 m = glm::translate(mat4(1), {H11_MID_X, 0.f, H11_ZC});
      m = glm::scale(m, {H11_LEN, 1.f, H11_DEPTH});
      draw(*mQuad, m, vp, 2); }

    // Tee box
    { mat4 m = glm::translate(mat4(1), {H11_TEE_X, 0.005f, H11_ZC});
      m = glm::scale(m, {1.5f, 1.f, H11_DEPTH - 0.2f});
      draw(*mQuad, m, vp, 1); }

    // ── Walls (concrete) ─────────────────────────────────────────────────────
    // North wall
    drawWall(vp, H11_MID_X, H11_WH * 0.5f, H11_ZN - H11_WTH * 0.5f,
             H11_LEN + H11_WTH * 2.f, H11_WH, H11_WTH);
    // South wall
    drawWall(vp, H11_MID_X, H11_WH * 0.5f, H11_ZS + H11_WTH * 0.5f,
             H11_LEN + H11_WTH * 2.f, H11_WH, H11_WTH);
    // West cap
    drawWall(vp, H11_XW - H11_WTH * 0.5f, H11_WH * 0.5f, H11_ZC,
             H11_WTH, H11_WH, H11_DEPTH + H11_WTH * 2.f);
    // East cap
    drawWall(vp, H11_XE + H11_WTH * 0.5f, H11_WH * 0.5f, H11_ZC,
             H11_WTH, H11_WH, H11_DEPTH + H11_WTH * 2.f);

    // ── Cup ───────────────────────────────────────────────────────────────────
    drawCup(vp, H11_CUP_X, H11_ZC);
}

void Hole11::wallCollide(vec3& pos, vec3& vel) const
{
    const float r  = 0.72f;
    const float BR = 0.08f;

    // Outer hole walls
    if (pos.z - BR < H11_ZN) { pos.z = H11_ZN + BR; if (vel.z < 0) vel.z =  fabsf(vel.z) * r; }
    if (pos.z + BR > H11_ZS) { pos.z = H11_ZS - BR; if (vel.z > 0) vel.z = -fabsf(vel.z) * r; }
    if (pos.x - BR < H11_XW) { pos.x = H11_XW + BR; if (vel.x < 0) vel.x =  fabsf(vel.x) * r; }
    if (pos.x + BR > H11_XE) { pos.x = H11_XE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r; }

    // Windmill base blocks: ball blocked from passing in X unless in the gap zone
    const float WM_XW  = 23.0f;   // west face of windmill
    const float WM_XE  = 25.0f;   // east face of windmill
    const float WM_GAP = 0.60f;   // gap half-width in z
    if (fabsf(pos.z - H11_ZC) > WM_GAP) {
        // Ball is outside the gap — windmill base blocks x passage
        if (vel.x > 0 && pos.x + BR > WM_XW && pos.x - BR < WM_XW) {
            pos.x = WM_XW - BR; vel.x = -fabsf(vel.x) * r;
        }
        if (vel.x < 0 && pos.x - BR < WM_XE && pos.x + BR > WM_XE) {
            pos.x = WM_XE + BR; vel.x =  fabsf(vel.x) * r;
        }
    }
}

bool Hole11::nearCup(const vec3& pos) const
{
    float dx = pos.x - H11_CUP_X, dz = pos.z - H11_ZC;
    return sqrtf(dx * dx + dz * dz) < 0.35f;
}

vec3 Hole11::getTeePos() const
{
    return { H11_TEE_X, BALL_R_CONST, H11_ZC };
}
