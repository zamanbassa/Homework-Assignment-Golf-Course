#include "Hole12.h"
#include <cmath>

static const float H12_TEE_X = H12_AXE - 2.0f;
static const float H12_TEE_Z = (H12_AZN + H12_AZS) * 0.5f;
static const float H12_CUP_X = H12_CXW + 2.0f;
static const float H12_CUP_Z = (H12_CZN + H12_CZS) * 0.5f;

Hole12::Hole12(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(12, q, b, c, t) {}

static void drawFloor12(const Mesh* quad, const mat4& vp,
                        float xw, float xe, float zn, float zs, int surf) {
    mat4 m = glm::translate(mat4(1), {(xw + xe) * 0.5f, 0.f, (zn + zs) * 0.5f});
    m = glm::scale(m, {xe - xw, 1.f, zs - zn});
    draw(*quad, m, vp, surf);
}

void Hole12::render(const mat4& vp) const {
    drawFloor12(mQuad, vp, H12_AXW, H12_AXE, H12_AZN, H12_AZS, 8);
    drawFloor12(mQuad, vp, H12_BXW, H12_BXE, H12_BZN, H12_BZS, 8);
    drawFloor12(mQuad, vp, H12_CXW, H12_CXE, H12_CZN, H12_CZS, 8);

    // tee
    { mat4 m = glm::translate(mat4(1), {H12_TEE_X, 0.005f, H12_TEE_Z});
      m = glm::scale(m, {1.5f, 1.f, (H12_AZS - H12_AZN) - 0.2f});
      draw(*mQuad, m, vp, 1); }

    const float WH = H12_WH, WTH = H12_WTH;

    drawWall(vp, (H12_AXW + H12_AXE) * 0.5f, WH * 0.5f, H12_AZN - WTH * 0.5f,
             (H12_AXE - H12_AXW) + WTH * 2.f, WH, WTH);
    drawWall(vp, H12_AXE + WTH * 0.5f, WH * 0.5f, (H12_AZN + H12_AZS) * 0.5f,
             WTH, WH, (H12_AZS - H12_AZN) + WTH * 2.f);
    drawWall(vp, (H12_BXE + H12_AXE) * 0.5f, WH * 0.5f, H12_AZS + WTH * 0.5f,
             (H12_AXE - H12_BXE) + WTH, WH, WTH);
    drawWall(vp, H12_BXE + WTH * 0.5f, WH * 0.5f, (H12_BZN + H12_CZS) * 0.5f,
             WTH, WH, (H12_CZS - H12_BZN) + WTH * 2.f);
    drawWall(vp, (H12_CXW + H12_CXE) * 0.5f, WH * 0.5f, H12_CZS + WTH * 0.5f,
             (H12_CXE - H12_CXW) + WTH * 2.f, WH, WTH);
    drawWall(vp, H12_CXW - WTH * 0.5f, WH * 0.5f, (H12_CZN + H12_CZS) * 0.5f,
             WTH, WH, (H12_CZS - H12_CZN) + WTH * 2.f);
    drawWall(vp, (H12_CXW + H12_BXW) * 0.5f, WH * 0.5f, H12_CZN - WTH * 0.5f,
             (H12_BXW - H12_CXW) + WTH, WH, WTH);
    drawWall(vp, H12_BXW - WTH * 0.5f, WH * 0.5f, (H12_AZN + H12_BZS) * 0.5f,
             WTH, WH, (H12_BZS - H12_AZN) + WTH * 2.f);

    drawCup(vp, H12_CUP_X, H12_CUP_Z);
}

void Hole12::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f;

    if (pos.x > H12_AXW && pos.x < H12_AXE && pos.z - BR < H12_AZN) {
        pos.z = H12_AZN + BR; if (vel.z < 0) vel.z =  fabsf(vel.z) * r;
    }
    if (pos.z > H12_AZN && pos.z < H12_AZS && pos.x + BR > H12_AXE) {
        pos.x = H12_AXE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r;
    }
    if (pos.x > H12_BXE && pos.x < H12_AXE && pos.z + BR > H12_AZS) {
        pos.z = H12_AZS - BR; if (vel.z > 0) vel.z = -fabsf(vel.z) * r;
    }
    if (pos.z > H12_BZN && pos.z < H12_CZS && pos.x + BR > H12_BXE) {
        pos.x = H12_BXE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r;
    }
    if (pos.x > H12_CXW && pos.x < H12_CXE && pos.z + BR > H12_CZS) {
        pos.z = H12_CZS - BR; if (vel.z > 0) vel.z = -fabsf(vel.z) * r;
    }
    if (pos.z > H12_CZN && pos.z < H12_CZS && pos.x - BR < H12_CXW) {
        pos.x = H12_CXW + BR; if (vel.x < 0) vel.x =  fabsf(vel.x) * r;
    }
    if (pos.x > H12_CXW && pos.x < H12_BXW && pos.z - BR < H12_CZN) {
        pos.z = H12_CZN + BR; if (vel.z < 0) vel.z =  fabsf(vel.z) * r;
    }
    if (pos.z > H12_AZN && pos.z < H12_BZS && pos.x - BR < H12_BXW) {
        pos.x = H12_BXW + BR; if (vel.x < 0) vel.x =  fabsf(vel.x) * r;
    }
}

bool Hole12::nearCup(const vec3& pos) const {
    float dx = pos.x - H12_CUP_X;
    float dz = pos.z - H12_CUP_Z;
    return sqrtf(dx * dx + dz * dz) < 0.35f;
}

vec3 Hole12::getTeePos() const {
    return { H12_TEE_X, BALL_R_CONST, H12_TEE_Z };
}
