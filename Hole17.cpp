#include "Hole17.h"
#include <cmath>

// Tee at south end of A arm; cup at west end of B arm
static const float H17_TEE_X = (H17_AXW + H17_AXE) * 0.5f;       // 24
static const float H17_TEE_Z = H17_AZS - 1.5f;                    // 42.5
static const float H17_CUP_X = H17_BXW + 2.0f;                    // 16 (west end of B)
static const float H17_CUP_Z = (H17_BZN + H17_BZS) * 0.5f;        // 40.5

// Bumper obstacles in B (west arm), column at inner L corner
static const float H17_B1X = 20.f, H17_B1Z = H17_BZN + 1.0f;     // near north wall (z=40)
static const float H17_B2X = 18.f, H17_B2Z = H17_BZS - 1.0f;     // near south wall (z=41)
static const float H17_COL_X = 23.f, H17_COL_Z = 41.f;            // inner corner of L

Hole17::Hole17(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(17, q, b, c, t) {}

static void drawSeg(const Mesh* quad, const mat4& vp,
                     float xw, float xe, float zn, float zs, int surf) {
    mat4 m = glm::translate(mat4(1), {(xw + xe) * 0.5f, 0.f, (zn + zs) * 0.5f});
    m = glm::scale(m, {xe - xw, 1.f, zs - zn});
    draw(*quad, m, vp, surf);
}

void Hole17::render(const mat4& vp) const {
    // ── B arm (west arm): grass → sand → concrete going west to east ──────
    // West (cup end) = grass, mid = sand, east (corner) = concrete
    drawSeg(mQuad, vp, H17_BXW, 18.0f,    H17_BZN, H17_BZS, 0);  // grass (west)
    drawSeg(mQuad, vp,  18.0f,  22.0f,    H17_BZN, H17_BZS, 4);  // sand (mid)
    drawSeg(mQuad, vp,  22.0f, H17_BXE,   H17_BZN, H17_BZS, 8);  // concrete (east)

    // A arm south section (below B arm): grass tee area
    drawSeg(mQuad, vp, H17_AXW, H17_AXE, H17_BZS, H17_AZS, 0);

    // Tee box marker
    { mat4 m = glm::translate(mat4(1), {H17_TEE_X, 0.005f, H17_TEE_Z});
      m = glm::scale(m, {(H17_AXE - H17_AXW) - 0.2f, 1.f, 1.5f});
      draw(*mQuad, m, vp, 1); }

    const float WH = H17_WH, WTH = H17_WTH;

    // ── L-shape walls (6 segments) ─────────────────────────────────────────
    // 1. North wall (z=BZN, x from BXW to AXE — shared by both arms)
    drawWall(vp, (H17_BXW + H17_AXE) * 0.5f, WH * 0.5f, H17_BZN - WTH * 0.5f,
             (H17_AXE - H17_BXW) + WTH * 2.f, WH, WTH);
    // 2. East wall (x=AXE, z from BZN to AZS — continuous for both arms)
    drawWall(vp, H17_AXE + WTH * 0.5f, WH * 0.5f, (H17_BZN + H17_AZS) * 0.5f,
             WTH, WH, (H17_AZS - H17_BZN) + WTH * 2.f);
    // 3. South wall of A arm (z=AZS, x from AXW to AXE)
    drawWall(vp, (H17_AXW + H17_AXE) * 0.5f, WH * 0.5f, H17_AZS + WTH * 0.5f,
             (H17_AXE - H17_AXW) + WTH * 2.f, WH, WTH);
    // 4. West wall of A below B (x=AXW, z from BZS to AZS)
    drawWall(vp, H17_AXW - WTH * 0.5f, WH * 0.5f, (H17_BZS + H17_AZS) * 0.5f,
             WTH, WH, (H17_AZS - H17_BZS) + WTH);
    // 5. South wall of B west of A (z=BZS, x from BXW to AXW)
    drawWall(vp, (H17_BXW + H17_AXW) * 0.5f, WH * 0.5f, H17_BZS + WTH * 0.5f,
             (H17_AXW - H17_BXW) + WTH, WH, WTH);
    // 6. West wall of B (x=BXW, z from BZN to BZS)
    drawWall(vp, H17_BXW - WTH * 0.5f, WH * 0.5f, (H17_BZN + H17_BZS) * 0.5f,
             WTH, WH, (H17_BZS - H17_BZN) + WTH * 2.f);

    // ── Two bumpers in the east arm ────────────────────────────────────────
    auto bump = [&](float bx, float bz) {
        mat4 m = glm::translate(mat4(1), {bx, 0.f, bz});
        m = glm::scale(m, {H17_BUMP * 2.f, WH * 1.4f, H17_BUMP * 2.f});
        draw(*mCylinder, m, vp, 10);
    };
    bump(H17_B1X, H17_B1Z);
    bump(H17_B2X, H17_B2Z);

    // ── Showcase stone column at the L corner ──────────────────────────────
    { mat4 m = glm::translate(mat4(1), {H17_COL_X, 0.f, H17_COL_Z});
      m = glm::scale(m, {H17_COL_R * 2.f, H17_COL_H, H17_COL_R * 2.f});
      draw(*mCylinder, m, vp, 8); }
    // Cap at top of the column for a finished look
    { mat4 m = glm::translate(mat4(1), {H17_COL_X, H17_COL_H, H17_COL_Z});
      m = glm::scale(m, {(H17_COL_R + 0.1f) * 2.f, 0.18f, (H17_COL_R + 0.1f) * 2.f});
      draw(*mCylinder, m, vp, 10); }

    // Cup
    drawCup(vp, H17_CUP_X, H17_CUP_Z);
}

void Hole17::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f;

    // 1. North wall (z=BZN, x in [BXW, AXE])
    if (pos.x > H17_BXW && pos.x < H17_AXE && pos.z - BR < H17_BZN) {
        pos.z = H17_BZN + BR; if (vel.z < 0) vel.z = fabsf(vel.z) * r;
    }
    // 2. East wall (x=AXE, z in [BZN, AZS])
    if (pos.z > H17_BZN && pos.z < H17_AZS && pos.x + BR > H17_AXE) {
        pos.x = H17_AXE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r;
    }
    // 3. South wall of A (z=AZS, x in [AXW, AXE])
    if (pos.x > H17_AXW && pos.x < H17_AXE && pos.z + BR > H17_AZS) {
        pos.z = H17_AZS - BR; if (vel.z > 0) vel.z = -fabsf(vel.z) * r;
    }
    // 4. West wall of A below B (x=AXW, z in [BZS, AZS])
    if (pos.z > H17_BZS && pos.z < H17_AZS && pos.x - BR < H17_AXW) {
        pos.x = H17_AXW + BR; if (vel.x < 0) vel.x = fabsf(vel.x) * r;
    }
    // 5. South wall of B west of A (z=BZS, x in [BXW, AXW])
    if (pos.x > H17_BXW && pos.x < H17_AXW && pos.z + BR > H17_BZS) {
        pos.z = H17_BZS - BR; if (vel.z > 0) vel.z = -fabsf(vel.z) * r;
    }
    // 6. West wall of B (x=BXW, z in [BZN, BZS])
    if (pos.z > H17_BZN && pos.z < H17_BZS && pos.x - BR < H17_BXW) {
        pos.x = H17_BXW + BR; if (vel.x < 0) vel.x = fabsf(vel.x) * r;
    }

    // Cylindrical obstacles (2 bumpers + 1 column)
    auto cyl = [&](float cx, float cz, float cr) {
        float dx = pos.x - cx, dz = pos.z - cz;
        float d2 = dx * dx + dz * dz;
        float minD = BR + cr;
        if (d2 < minD * minD && d2 > 0.0001f) {
            float d  = sqrtf(d2);
            float nx = dx / d, nz = dz / d;
            pos.x += nx * (minD - d);
            pos.z += nz * (minD - d);
            float vdn = vel.x * nx + vel.z * nz;
            if (vdn < 0) { vel.x -= (1.f + r) * vdn * nx; vel.z -= (1.f + r) * vdn * nz; }
        }
    };
    cyl(H17_B1X,  H17_B1Z,  H17_BUMP);
    cyl(H17_B2X,  H17_B2Z,  H17_BUMP);
    cyl(H17_COL_X, H17_COL_Z, H17_COL_R);
}

bool Hole17::nearCup(const vec3& pos) const {
    float dx = pos.x - H17_CUP_X;
    float dz = pos.z - H17_CUP_Z;
    return sqrtf(dx * dx + dz * dz) < 0.35f;
}

vec3 Hole17::getTeePos() const {
    return { H17_TEE_X, 0.08f, H17_TEE_Z };
}

const std::vector<glm::vec3>& Hole17::lampPositions() {
    static const std::vector<glm::vec3> pts = {
        {27.5f, 0.f, 43.5f},  // east of A arm (near tee)
        {27.5f, 0.f, 40.0f},  // east of corner/column
        {20.0f, 0.f, 38.0f},  // mid B arm north
        {15.0f, 0.f, 41.5f},  // west of cup
    };
    return pts;
}
