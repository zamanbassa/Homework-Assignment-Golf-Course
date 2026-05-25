#include "Hole18.h"
#include <cmath>

static const float H18_TEE_X = 13.5f;
static const float H18_TEE_Z = 46.0f;
static const float H18_CUP_X = 18.5f;
static const float H18_CUP_Z = 52.0f;

Hole18::Hole18(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(18, q, b, c, t) {}

void Hole18::render(const mat4& vp) const {
    const float midX = (H18_XW + H18_XE) * 0.5f;
    const float midZ = (H18_ZN + H18_ZS) * 0.5f;
    const float W    = H18_XE - H18_XW;
    const float D    = H18_ZS - H18_ZN;

    { mat4 m = glm::translate(mat4(1), {midX, 0.f, midZ});
      m = glm::scale(m, {W, 1.f, D});
      draw(*mQuad, m, vp, 8); }

    {
        const float pad = 0.6f;
        { mat4 m = glm::translate(mat4(1), {midX, -0.002f, H18_ZN - pad * 0.5f});
          m = glm::scale(m, {W + 2.f * pad, 1.f, pad});
          draw(*mQuad, m, vp, 0); }
        { mat4 m = glm::translate(mat4(1), {midX, -0.002f, H18_ZS + pad * 0.5f});
          m = glm::scale(m, {W + 2.f * pad, 1.f, pad});
          draw(*mQuad, m, vp, 0); }
        { mat4 m = glm::translate(mat4(1), {H18_XW - pad * 0.5f, -0.002f, midZ});
          m = glm::scale(m, {pad, 1.f, D});
          draw(*mQuad, m, vp, 0); }
        { mat4 m = glm::translate(mat4(1), {H18_XE + pad * 0.5f, -0.002f, midZ});
          m = glm::scale(m, {pad, 1.f, D});
          draw(*mQuad, m, vp, 0); }
    }

    // tee
    { mat4 m = glm::translate(mat4(1), {H18_TEE_X, 0.006f, H18_TEE_Z});
      m = glm::scale(m, {2.4f, 1.f, 1.4f});
      draw(*mQuad, m, vp, 1); }

    const float WH = H18_WH, WTH = H18_WTH;
    drawWall(vp, midX, WH * 0.5f, H18_ZN - WTH * 0.5f, W + WTH * 2.f, WH, WTH);
    drawWall(vp, midX, WH * 0.5f, H18_ZS + WTH * 0.5f, W + WTH * 2.f, WH, WTH);
    drawWall(vp, H18_XW - WTH * 0.5f, WH * 0.5f, midZ, WTH, WH, D + WTH * 2.f);
    drawWall(vp, H18_XE + WTH * 0.5f, WH * 0.5f, midZ, WTH, WH, D + WTH * 2.f);

    auto pillar = [&](float px, float pz) {
        mat4 m = glm::translate(mat4(1), {px, 0.f, pz});
        m = glm::scale(m, {0.55f, 1.4f, 0.55f});
        draw(*mCylinder, m, vp, 10);
    };
    pillar(H18_XW, H18_ZN); pillar(H18_XE, H18_ZN);
    pillar(H18_XW, H18_ZS); pillar(H18_XE, H18_ZS);

    // lighthouse
    { mat4 m = glm::translate(mat4(1), {H18_LH_X, 0.f, H18_LH_Z});
      m = glm::scale(m, {H18_LH_R * 2.f, H18_LH_H, H18_LH_R * 2.f});
      draw(*mCylinder, m, vp, 8); }

    { mat4 m = glm::translate(mat4(1), {H18_LH_X, H18_LH_H * 0.55f, H18_LH_Z});
      m = glm::scale(m, {(H18_LH_R + 0.05f) * 2.f, 0.35f, (H18_LH_R + 0.05f) * 2.f});
      draw(*mCylinder, m, vp, 21); }

    { mat4 m = glm::translate(mat4(1), {H18_LH_X, H18_LH_H, H18_LH_Z});
      m = glm::scale(m, {H18_LANT_R * 2.f, H18_LANT_H, H18_LANT_R * 2.f});
      draw(*mCylinder, m, vp, 14); }

    { mat4 m = glm::translate(mat4(1), {H18_LH_X, H18_LH_H + H18_LANT_H, H18_LH_Z});
      m = glm::scale(m, {(H18_LANT_R + 0.15f) * 2.f, H18_ROOF_H, (H18_LANT_R + 0.15f) * 2.f});
      draw(*mCylinder, m, vp, 10); }

    { mat4 m = glm::translate(mat4(1), {H18_LH_X, H18_LH_H + H18_LANT_H + H18_ROOF_H, H18_LH_Z});
      m = glm::scale(m, {0.08f, 0.7f, 0.08f});
      draw(*mCylinder, m, vp, 16); }

    drawCup(vp, H18_CUP_X, H18_CUP_Z);
}

void Hole18::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f;

    if (pos.x - BR < H18_XW) { pos.x = H18_XW + BR; if (vel.x < 0) vel.x =  fabsf(vel.x) * r; }
    if (pos.x + BR > H18_XE) { pos.x = H18_XE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r; }
    if (pos.z - BR < H18_ZN) { pos.z = H18_ZN + BR; if (vel.z < 0) vel.z =  fabsf(vel.z) * r; }
    if (pos.z + BR > H18_ZS) { pos.z = H18_ZS - BR; if (vel.z > 0) vel.z = -fabsf(vel.z) * r; }

    float dx = pos.x - H18_LH_X, dz = pos.z - H18_LH_Z;
    float d2 = dx * dx + dz * dz;
    float minD = BR + H18_LH_R;
    if (d2 < minD * minD && d2 > 0.0001f) {
        float d  = sqrtf(d2);
        float nx = dx / d, nz = dz / d;
        pos.x += nx * (minD - d);
        pos.z += nz * (minD - d);
        float vdn = vel.x * nx + vel.z * nz;
        if (vdn < 0) { vel.x -= (1.f + r) * vdn * nx; vel.z -= (1.f + r) * vdn * nz; }
    }

    auto pillar = [&](float cx, float cz) {
        float dx = pos.x - cx, dz = pos.z - cz;
        float d2 = dx * dx + dz * dz;
        const float pr = 0.55f;
        float minD = BR + pr;
        if (d2 < minD * minD && d2 > 0.0001f) {
            float d  = sqrtf(d2);
            float nx = dx / d, nz = dz / d;
            pos.x += nx * (minD - d);
            pos.z += nz * (minD - d);
            float vdn = vel.x * nx + vel.z * nz;
            if (vdn < 0) { vel.x -= (1.f + r) * vdn * nx; vel.z -= (1.f + r) * vdn * nz; }
        }
    };
    pillar(H18_XW, H18_ZN); pillar(H18_XE, H18_ZN);
    pillar(H18_XW, H18_ZS); pillar(H18_XE, H18_ZS);
}

bool Hole18::nearCup(const vec3& pos) const {
    float dx = pos.x - H18_CUP_X;
    float dz = pos.z - H18_CUP_Z;
    return sqrtf(dx * dx + dz * dz) < 0.40f;
}

vec3 Hole18::getTeePos() const {
    return { H18_TEE_X, 0.08f, H18_TEE_Z };
}

const std::vector<glm::vec3>& Hole18::lampPositions() {
    static const std::vector<glm::vec3> pts = {
        {H18_XW - 1.5f, 0.f, H18_ZN - 1.0f},
        {H18_XE + 1.5f, 0.f, H18_ZN - 1.0f},
        {H18_XW - 1.5f, 0.f, H18_ZS - 1.5f},
        {H18_XE + 1.5f, 0.f, H18_ZS - 1.5f},
    };
    return pts;
}

const std::vector<glm::vec3>& Hole18::treePositions() {
    static const std::vector<glm::vec3> pts = {
        {H18_XW - 2.5f, 0.f, H18_ZN + 1.0f},
        {H18_XW - 2.5f, 0.f, H18_ZS - 2.0f},
        {H18_XE + 2.5f, 0.f, H18_ZN + 1.5f},
        {H18_XE + 2.5f, 0.f, H18_ZS - 2.0f},
    };
    return pts;
}
