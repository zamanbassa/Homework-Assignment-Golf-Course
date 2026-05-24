#include "Hole13.h"
#include <cmath>

// Tee at east end of top arm A, cup at west end of bottom arm C
static const float H13_TEE_X = H13_AXE - 2.0f;                       // 38.0
static const float H13_TEE_Z = (H13_AZN + H13_AZS) * 0.5f;           // -11.0
static const float H13_CUP_X = H13_CXW + 2.0f;                       // 32.0
static const float H13_CUP_Z = (H13_CZN + H13_CZS) * 0.5f;           // -5.0

Hole13::Hole13(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(13, q, b, c, t) {}

static void drawFloor(const Mesh* quad, const mat4& vp,
                      float xw, float xe, float zn, float zs, int surf) {
    mat4 m = glm::translate(mat4(1), {(xw + xe) * 0.5f, 0.f, (zn + zs) * 0.5f});
    m = glm::scale(m, {xe - xw, 1.f, zs - zn});
    draw(*quad, m, vp, surf);
}

void Hole13::render(const mat4& vp) const {
    // Three concrete floor segments forming a Z
    drawFloor(mQuad, vp, H13_AXW, H13_AXE, H13_AZN, H13_AZS, 8);  // A — top arm
    drawFloor(mQuad, vp, H13_BXW, H13_BXE, H13_BZN, H13_BZS, 8);  // B — connector
    drawFloor(mQuad, vp, H13_CXW, H13_CXE, H13_CZN, H13_CZS, 8);  // C — bottom arm

    // Tee box (in A, east end)
    { mat4 m = glm::translate(mat4(1), {H13_TEE_X, 0.005f, H13_TEE_Z});
      m = glm::scale(m, {1.5f, 1.f, (H13_AZS - H13_AZN) - 0.2f});
      draw(*mQuad, m, vp, 1); }

    const float WH = H13_WH, WTH = H13_WTH;

    // Walls — external boundary of A∪B∪C going clockwise

    // W1: north wall of A   (z=-12, x in [34,40])
    drawWall(vp, (H13_AXW + H13_AXE) * 0.5f, WH * 0.5f, H13_AZN - WTH * 0.5f,
             (H13_AXE - H13_AXW) + WTH * 2.f, WH, WTH);

    // W2: east wall of A   (x=40, z in [-12,-10])
    drawWall(vp, H13_AXE + WTH * 0.5f, WH * 0.5f, (H13_AZN + H13_AZS) * 0.5f,
             WTH, WH, (H13_AZS - H13_AZN) + WTH * 2.f);

    // W3: south wall of A east of B   (z=-10, x in [36,40])
    drawWall(vp, (H13_BXE + H13_AXE) * 0.5f, WH * 0.5f, H13_AZS + WTH * 0.5f,
             (H13_AXE - H13_BXE) + WTH, WH, WTH);

    // W4: east wall of B+C   (x=36, z in [-10,-4])
    drawWall(vp, H13_BXE + WTH * 0.5f, WH * 0.5f, (H13_BZN + H13_CZS) * 0.5f,
             WTH, WH, (H13_CZS - H13_BZN) + WTH * 2.f);

    // W5: south wall of C   (z=-4, x in [30,36])
    drawWall(vp, (H13_CXW + H13_CXE) * 0.5f, WH * 0.5f, H13_CZS + WTH * 0.5f,
             (H13_CXE - H13_CXW) + WTH * 2.f, WH, WTH);

    // W6: west wall of C   (x=30, z in [-6,-4])
    drawWall(vp, H13_CXW - WTH * 0.5f, WH * 0.5f, (H13_CZN + H13_CZS) * 0.5f,
             WTH, WH, (H13_CZS - H13_CZN) + WTH * 2.f);

    // W7: north wall of C west of B   (z=-6, x in [30,34])
    drawWall(vp, (H13_CXW + H13_BXW) * 0.5f, WH * 0.5f, H13_CZN - WTH * 0.5f,
             (H13_BXW - H13_CXW) + WTH, WH, WTH);

    // W8: west wall of B+A   (x=34, z in [-12,-6])
    drawWall(vp, H13_BXW - WTH * 0.5f, WH * 0.5f, (H13_AZN + H13_BZS) * 0.5f,
             WTH, WH, (H13_BZS - H13_AZN) + WTH * 2.f);

    // Cup (in C, west end)
    drawCup(vp, H13_CUP_X, H13_CUP_Z);
}

void Hole13::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f;

    // W1: north of A (z=-12, x in [34,40]) — bounce south
    if (pos.x > H13_AXW && pos.x < H13_AXE && pos.z - BR < H13_AZN) {
        pos.z = H13_AZN + BR; if (vel.z < 0) vel.z =  fabsf(vel.z) * r;
    }
    // W2: east of A (x=40, z in [-12,-10]) — bounce west
    if (pos.z > H13_AZN && pos.z < H13_AZS && pos.x + BR > H13_AXE) {
        pos.x = H13_AXE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r;
    }
    // W3: south of A east of B (z=-10, x in [36,40]) — bounce north
    if (pos.x > H13_BXE && pos.x < H13_AXE && pos.z + BR > H13_AZS) {
        pos.z = H13_AZS - BR; if (vel.z > 0) vel.z = -fabsf(vel.z) * r;
    }
    // W4: east of B+C (x=36, z in [-10,-4]) — bounce west
    if (pos.z > H13_BZN && pos.z < H13_CZS && pos.x + BR > H13_BXE) {
        pos.x = H13_BXE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r;
    }
    // W5: south of C (z=-4, x in [30,36]) — bounce north
    if (pos.x > H13_CXW && pos.x < H13_CXE && pos.z + BR > H13_CZS) {
        pos.z = H13_CZS - BR; if (vel.z > 0) vel.z = -fabsf(vel.z) * r;
    }
    // W6: west of C (x=30, z in [-6,-4]) — bounce east
    if (pos.z > H13_CZN && pos.z < H13_CZS && pos.x - BR < H13_CXW) {
        pos.x = H13_CXW + BR; if (vel.x < 0) vel.x =  fabsf(vel.x) * r;
    }
    // W7: north of C west of B (z=-6, x in [30,34]) — bounce south
    if (pos.x > H13_CXW && pos.x < H13_BXW && pos.z - BR < H13_CZN) {
        pos.z = H13_CZN + BR; if (vel.z < 0) vel.z =  fabsf(vel.z) * r;
    }
    // W8: west of B+A (x=34, z in [-12,-6]) — bounce east
    if (pos.z > H13_AZN && pos.z < H13_BZS && pos.x - BR < H13_BXW) {
        pos.x = H13_BXW + BR; if (vel.x < 0) vel.x =  fabsf(vel.x) * r;
    }
}

bool Hole13::nearCup(const vec3& pos) const {
    float dx = pos.x - H13_CUP_X;
    float dz = pos.z - H13_CUP_Z;
    return sqrtf(dx * dx + dz * dz) < 0.35f;
}

vec3 Hole13::getTeePos() const {
    return { H13_TEE_X, 0.08f, H13_TEE_Z };
}
