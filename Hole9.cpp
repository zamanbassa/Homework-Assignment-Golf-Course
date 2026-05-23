#include "Hole9.h"
#include <cmath>

static const float PI9 = 3.14159265358979f;

// ── Tee / Cup world positions ─────────────────────────────────────────────────
static const float H9_TEE_X = (H9_WX1 + H9_WX2) * 0.5f;   // 25.0
static const float H9_CUP_X = (H9_EX1 + H9_EX2) * 0.5f;   // 31.0
static const float H9_PLAY_Z = H9_ZN + 1.5f;               // -26.5  (tee/cup z)

// ── Constructor ───────────────────────────────────────────────────────────────
Hole9::Hole9(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(9, q, b, c, t) {}

// ── render ────────────────────────────────────────────────────────────────────
void Hole9::render(const mat4& vp) const {
    // ── Fairway floors ──────────────────────────────────────────────────────
    const float armW  = H9_WX2 - H9_WX1;                    // 4
    const float armL  = H9_ZS  - H9_ZN;                     // 12
    const float connW = H9_EX2 - H9_WX1;                    // 10
    const float connL = H9_ZB  - H9_ZS;                     //  3
    const float armMidZ = (H9_ZN + H9_ZS) * 0.5f;           // -34
    const float connMidZ = (H9_ZS + H9_ZB) * 0.5f;          // -41.5
    const float connMidX = (H9_WX1 + H9_EX2) * 0.5f;        //  28

    {   // west arm floor
        mat4 m = glm::translate(mat4(1), {H9_TEE_X, 0.f, armMidZ});
        m = glm::scale(m, {armW, 1.f, armL});
        draw(*mQuad, m, vp, 0);
    }
    {   // east arm floor
        mat4 m = glm::translate(mat4(1), {H9_CUP_X, 0.f, armMidZ});
        m = glm::scale(m, {armW, 1.f, armL});
        draw(*mQuad, m, vp, 0);
    }
    {   // connector floor
        mat4 m = glm::translate(mat4(1), {connMidX, 0.f, connMidZ});
        m = glm::scale(m, {connW, 1.f, connL});
        draw(*mQuad, m, vp, 0);
    }

    // ── Tee box ─────────────────────────────────────────────────────────────
    {
        mat4 m = glm::translate(mat4(1), {H9_TEE_X, 0.005f, H9_PLAY_Z});
        m = glm::scale(m, {armW - 0.2f, 1.f, 1.2f});
        draw(*mQuad, m, vp, 1);
    }

    // ── Outer walls ─────────────────────────────────────────────────────────
    const float fullH = H9_ZB - H9_ZN + H9_WTH * 2.f;      // total N-S span + caps
    // west outer
    drawWall(vp, H9_WX1 - H9_WTH * 0.5f,  H9_WH * 0.5f,
             (H9_ZN + H9_ZB) * 0.5f,
             H9_WTH, H9_WH, fullH);
    // east outer
    drawWall(vp, H9_EX2 + H9_WTH * 0.5f,  H9_WH * 0.5f,
             (H9_ZN + H9_ZB) * 0.5f,
             H9_WTH, H9_WH, fullH);
    // south outer
    drawWall(vp, connMidX, H9_WH * 0.5f,
             H9_ZB + H9_WTH * 0.5f,
             connW + H9_WTH * 2.f, H9_WH, H9_WTH);

    // ── North end-walls (seal top of each arm) ───────────────────────────────
    drawWall(vp, H9_TEE_X, H9_WH * 0.5f,
             H9_ZN - H9_WTH * 0.5f,
             armW + H9_WTH * 2.f, H9_WH, H9_WTH);
    drawWall(vp, H9_CUP_X, H9_WH * 0.5f,
             H9_ZN - H9_WTH * 0.5f,
             armW + H9_WTH * 2.f, H9_WH, H9_WTH);

    // ── Inner divider (blocks direct passage between arms above connector) ──
    const float divW  = H9_EX1 - H9_WX2;                    // 2
    const float divL  = -(H9_ZS - H9_ZN) + H9_WTH * 2.f;   // arm length + caps
    drawWall(vp, (H9_WX2 + H9_EX1) * 0.5f, H9_WH * 0.5f,
             armMidZ,
             divW, H9_WH, -(armL + H9_WTH * 2.f));

    // ── Corner bumpers: visible cylinders at inner arm-connector corners ─────
    auto drawBump = [&](float bx, float bz) {
        mat4 m = glm::translate(mat4(1), {bx, 0.f, bz});
        m = glm::scale(m, {H9_BUMP * 2.f, H9_WH, H9_BUMP * 2.f});
        draw(*mCylinder, m, vp, 10);
    };
    drawBump(H9_WX2, H9_ZS);
    drawBump(H9_EX1, H9_ZS);

    // ── Cup ─────────────────────────────────────────────────────────────────
    drawCup(vp, H9_CUP_X, H9_PLAY_Z);
}

// ── wallCollide ───────────────────────────────────────────────────────────────
void Hole9::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f;  // BALL_R

    // Outer west/east walls (always active)
    if (pos.x - BR < H9_WX1) { pos.x = H9_WX1 + BR; if (vel.x < 0) vel.x =  fabsf(vel.x)*r; }
    if (pos.x + BR > H9_EX2) { pos.x = H9_EX2 - BR; if (vel.x > 0) vel.x = -fabsf(vel.x)*r; }

    // North wall
    if (pos.z - BR < H9_ZN) { pos.z = H9_ZN + BR; if (vel.z < 0) vel.z = fabsf(vel.z)*r; }

    // South wall
    if (pos.z + BR > H9_ZB) { pos.z = H9_ZB - BR; if (vel.z > 0) vel.z = -fabsf(vel.z)*r; }

    // Inner divider — only relevant when ball is above the connector
    if (pos.z > H9_ZS) {
        const float mid = (H9_WX2 + H9_EX1) * 0.5f;   // 28
        if (pos.x <= mid) {
            // ball is in (or trying to enter) the west arm side
            if (pos.x + BR > H9_WX2) {
                pos.x = H9_WX2 - BR;
                if (vel.x > 0) vel.x = -fabsf(vel.x)*r;
            }
        } else {
            // ball is in (or trying to enter) the east arm side
            if (pos.x - BR < H9_EX1) {
                pos.x = H9_EX1 + BR;
                if (vel.x < 0) vel.x = fabsf(vel.x)*r;
            }
        }
    }

    // Corner bumpers (cylindrical collision)
    auto bump = [&](float bx, float bz) {
        float dx = pos.x - bx, dz = pos.z - bz;
        float d2 = dx*dx + dz*dz;
        float minD = BR + H9_BUMP;
        if (d2 < minD*minD && d2 > 0.0001f) {
            float d  = sqrtf(d2);
            float nx = dx/d, nz = dz/d;
            float push = minD - d;
            pos.x += nx*push;  pos.z += nz*push;
            float vdn = vel.x*nx + vel.z*nz;
            if (vdn < 0) { vel.x -= (1.f+r)*vdn*nx;  vel.z -= (1.f+r)*vdn*nz; }
        }
    };
    bump(H9_WX2, H9_ZS);
    bump(H9_EX1, H9_ZS);
}

// ── nearCup ───────────────────────────────────────────────────────────────────
bool Hole9::nearCup(const vec3& pos) const {
    float dx = pos.x - H9_CUP_X;
    float dz = pos.z - H9_PLAY_Z;
    return sqrtf(dx*dx + dz*dz) < 0.35f;
}

// ── getTeePos ─────────────────────────────────────────────────────────────────
vec3 Hole9::getTeePos() const {
    return { H9_TEE_X, 0.08f, H9_PLAY_Z };
}