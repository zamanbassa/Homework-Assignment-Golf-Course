#include "Hole15.h"
#include <cmath>

static const float H15_LEN    = H15_ZS - H15_ZN;
static const float H15_MID_Z  = (H15_ZN + H15_ZS) * 0.5f;
static const float H15_TEE_Z  = H15_ZN + 2.0f;
static const float H15_CUP_Z  = H15_ZS - 2.0f;

static const float H15_C1X = H15_CX - 0.6f, H15_C1Z = 21.0f;
static const float H15_C2X = H15_CX + 0.6f, H15_C2Z = 23.5f;
static const float H15_C3X = H15_CX - 0.6f, H15_C3Z = 26.0f;
static const float H15_C4X = H15_CX + 0.6f, H15_C4Z = 28.5f;

Hole15::Hole15(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(15, q, b, c, t) {}

void Hole15::render(const mat4& vp) const {
    { mat4 m = glm::translate(mat4(1), {H15_CX, 0.f, H15_MID_Z});
      m = glm::scale(m, {H15_FW, 1.f, H15_LEN});
      draw(*mQuad, m, vp, 3); }

    // Tee box
    { mat4 m = glm::translate(mat4(1), {H15_CX, 0.005f, H15_TEE_Z});
      m = glm::scale(m, {H15_FW - 0.2f, 1.f, 1.5f});
      draw(*mQuad, m, vp, 1); }

    drawWall(vp, H15_XE + H15_WTH * 0.5f, H15_WH * 0.5f, H15_MID_Z,
             H15_WTH, H15_WH, H15_LEN + H15_WTH * 2.f);

    drawWall(vp, H15_XW - H15_WTH * 0.5f, H15_WH * 0.5f, H15_MID_Z,
             H15_WTH, H15_WH, H15_LEN + H15_WTH * 2.f);

    drawWall(vp, H15_CX, H15_WH * 0.5f, H15_ZN - H15_WTH * 0.5f,
             H15_FW + H15_WTH * 2.f, H15_WH, H15_WTH);

    drawWall(vp, H15_CX, H15_WH * 0.5f, H15_ZS + H15_WTH * 0.5f,
             H15_FW + H15_WTH * 2.f, H15_WH, H15_WTH);

    auto drawCol = [&](float cx, float cz) {
        mat4 m = glm::translate(mat4(1), {cx, 0.f, cz});
        m = glm::scale(m, {H15_COL_R * 2.f, H15_COL_H, H15_COL_R * 2.f});
        draw(*mCylinder, m, vp, 8);
    };
    drawCol(H15_C1X, H15_C1Z);
    drawCol(H15_C2X, H15_C2Z);
    drawCol(H15_C3X, H15_C3Z);
    drawCol(H15_C4X, H15_C4Z);

    drawCup(vp, H15_CX, H15_CUP_Z);
}

void Hole15::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f;

    if (pos.x - BR < H15_XW) { pos.x = H15_XW + BR; if (vel.x < 0) vel.x =  fabsf(vel.x) * r; }
    if (pos.x + BR > H15_XE) { pos.x = H15_XE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r; }
    if (pos.z - BR < H15_ZN) { pos.z = H15_ZN + BR; if (vel.z < 0) vel.z =  fabsf(vel.z) * r; }
    if (pos.z + BR > H15_ZS) { pos.z = H15_ZS - BR; if (vel.z > 0) vel.z = -fabsf(vel.z) * r; }

    auto col = [&](float cx, float cz) {
        float dx = pos.x - cx, dz = pos.z - cz;
        float d2 = dx * dx + dz * dz;
        float minD = BR + H15_COL_R;
        if (d2 < minD * minD && d2 > 0.0001f) {
            float d  = sqrtf(d2);
            float nx = dx / d, nz = dz / d;
            pos.x += nx * (minD - d);
            pos.z += nz * (minD - d);
            float vdn = vel.x * nx + vel.z * nz;
            if (vdn < 0) { vel.x -= (1.f + r) * vdn * nx; vel.z -= (1.f + r) * vdn * nz; }
        }
    };
    col(H15_C1X, H15_C1Z);
    col(H15_C2X, H15_C2Z);
    col(H15_C3X, H15_C3Z);
    col(H15_C4X, H15_C4Z);
}

bool Hole15::nearCup(const vec3& pos) const {
    float dx = pos.x - H15_CX;
    float dz = pos.z - H15_CUP_Z;
    return sqrtf(dx * dx + dz * dz) < 0.35f;
}

vec3 Hole15::getTeePos() const {
    return { H15_CX, 0.08f, H15_TEE_Z };
}
