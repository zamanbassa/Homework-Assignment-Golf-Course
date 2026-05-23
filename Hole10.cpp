#include "Hole10.h"
#include <cmath>
#include <vector>

static const float PI10 = 3.14159265358979f;

// ── Derived constants ─────────────────────────────────────────────────────────
static const float H10_RMID   = (H10_RI + H10_RO) * 0.5f; // 4.0
// Tee at angle=0 (east), cup at angle=3π/2 (north, -z from centre)
static const float H10_TEE_X  = H10_CX + H10_RMID;         // 38.0
static const float H10_TEE_Z  = H10_CZ;                     // -35.0
static const float H10_CUP_X  = H10_CX;                     // 34.0
static const float H10_CUP_Z  = H10_CZ - H10_RMID;         // -39.0
// Channel bumper at the west apex (angle=π, mid-radius)
static const float H10_BUMP_X = H10_CX - H10_RMID;         // 30.0
static const float H10_BUMP_Z = H10_CZ;                     // -35.0

// ── Local mesh helpers (mirrors main.cpp — upload() is non-static there) ─────
static Mesh h10_makeArcFloor(float Ri, float Ro,
                              float tStart, float tEnd, int N = 48) {
    std::vector<Vertex> V;
    std::vector<unsigned> I;
    float dt = (tEnd - tStart) / N;
    for (int i = 0; i <= N; i++) {
        float t = tStart + i * dt;
        float ct = cosf(t), st = sinf(t);
        float u = (float)i / N;
        V.push_back({{Ri * ct, 0, Ri * st}, {0, 1, 0}, {u, 0}});
        V.push_back({{Ro * ct, 0, Ro * st}, {0, 1, 0}, {u, 1}});
    }
    for (int i = 0; i < N; i++) {
        unsigned b = 2 * i;
        I.insert(I.end(), {b, b + 2, b + 1, b + 1, b + 2, b + 3});
    }
    return upload(V, I);
}

static Mesh h10_makeArcWall(float R, float WTH, float WH,
                             float tStart, float tEnd, int N = 48) {
    std::vector<Vertex> V;
    std::vector<unsigned> I;
    float Ro = R + WTH * 0.5f, Ri = R - WTH * 0.5f;
    float dt = (tEnd - tStart) / N;
    for (int i = 0; i <= N; i++) {
        float t  = tStart + i * dt;
        float ct = cosf(t), st = sinf(t);
        vec3 no  = { ct, 0,  st};
        vec3 ni  = {-ct, 0, -st};
        V.push_back({{Ro * ct,   0, Ro * st}, no, {(float)i / N, 0}});
        V.push_back({{Ro * ct, WH, Ro * st}, no, {(float)i / N, 1}});
        V.push_back({{Ri * ct,   0, Ri * st}, ni, {(float)i / N, 0}});
        V.push_back({{Ri * ct, WH, Ri * st}, ni, {(float)i / N, 1}});
    }
    for (int i = 0; i < N; i++) {
        unsigned b = 4 * i;
        I.insert(I.end(), {b,     b + 1, b + 4, b + 1, b + 5, b + 4}); // outer
        I.insert(I.end(), {b + 2, b + 6, b + 3, b + 3, b + 6, b + 7}); // inner
        I.insert(I.end(), {b + 1, b + 3, b + 5, b + 3, b + 7, b + 5}); // top cap
    }
    return upload(V, I);
}

// ── Constructor ───────────────────────────────────────────────────────────────
Hole10::Hole10(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(10, q, b, c, t)
{
    // 270° arc: t=0 (east/tee) → t=3π/2 (south/cup), counterclockwise
    const float ARC_END = 3.0f * PI10 / 2.0f;
    mArcFloor   = h10_makeArcFloor(H10_RI, H10_RO, 0.0f, ARC_END, 56);
    mArcWallIn  = h10_makeArcWall(H10_RI, H10_WTH, H10_WH, 0.0f, ARC_END, 56);
    mArcWallOut = h10_makeArcWall(H10_RO, H10_WTH, H10_WH, 0.0f, ARC_END, 56);
}

// ── Destructor ────────────────────────────────────────────────────────────────
Hole10::~Hole10() {
    glDeleteBuffers(1, &mArcFloor.vbo);
    glDeleteBuffers(1, &mArcFloor.ebo);
    glDeleteVertexArrays(1, &mArcFloor.vao);

    glDeleteBuffers(1, &mArcWallIn.vbo);
    glDeleteBuffers(1, &mArcWallIn.ebo);
    glDeleteVertexArrays(1, &mArcWallIn.vao);

    glDeleteBuffers(1, &mArcWallOut.vbo);
    glDeleteBuffers(1, &mArcWallOut.ebo);
    glDeleteVertexArrays(1, &mArcWallOut.vao);
}

// ── render ────────────────────────────────────────────────────────────────────
void Hole10::render(const mat4& vp) const {
    const float chanW = H10_RO - H10_RI; // 4.0

    // ── Arc floor ───────────────────────────────────────────────────────────
    {
        mat4 m = glm::translate(mat4(1), {H10_CX, 0.0f, H10_CZ});
        draw(mArcFloor, m, vp, 0);
    }

    // ── Tee box (east opening) ───────────────────────────────────────────────
    {
        mat4 m = glm::translate(mat4(1), {H10_TEE_X, 0.005f, H10_TEE_Z});
        m = glm::scale(m, {chanW - 0.2f, 1.f, 1.5f});
        draw(*mQuad, m, vp, 1);
    }

    // ── Arc walls ───────────────────────────────────────────────────────────
    {
        mat4 m = glm::translate(mat4(1), {H10_CX, 0.0f, H10_CZ});
        draw(mArcWallIn,  m, vp, 8);
        draw(mArcWallOut, m, vp, 8);
    }

    // ── End cap at angle=0 (tee end, east): seals gap on z=H10_CZ line ──────
    drawWall(vp,
             H10_CX + H10_RMID,       H10_WH * 0.5f, H10_CZ,
             chanW + H10_WTH * 2.f,   H10_WH,         H10_WTH);

    // ── End cap at angle=3π/2 (cup end, south): seals gap on x=H10_CX line ─
    drawWall(vp,
             H10_CX,                  H10_WH * 0.5f, H10_CZ - H10_RMID,
             H10_WTH,                 H10_WH,         chanW + H10_WTH * 2.f);

    // ── Central rock pillar (decorative, inside inner radius) ────────────────
    {
        mat4 m = glm::translate(mat4(1), {H10_CX, 0.0f, H10_CZ});
        m = glm::scale(m, {H10_ROCK_R * 2.f, H10_WH * 1.8f, H10_ROCK_R * 2.f});
        draw(*mCylinder, m, vp, 10);
    }

    // ── Channel bumper at west apex (angle=π, mid-radius) ───────────────────
    {
        mat4 m = glm::translate(mat4(1), {H10_BUMP_X, 0.0f, H10_BUMP_Z});
        m = glm::scale(m, {H10_BUMP_R * 2.f, H10_WH, H10_BUMP_R * 2.f});
        draw(*mCylinder, m, vp, 10);
    }

    // ── Cup at south end ─────────────────────────────────────────────────────
    drawCup(vp, H10_CUP_X, H10_CUP_Z);
}

// ── wallCollide ───────────────────────────────────────────────────────────────
void Hole10::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f; // BALL_R
    const float cx = H10_CX, cz = H10_CZ;
    const float ri = H10_RI, ro = H10_RO;

    float dx = pos.x - cx, dz = pos.z - cz;
    float d2 = dx * dx + dz * dz;
    if (d2 < 0.001f) return;
    float d  = sqrtf(d2);
    float nx = dx / d, nz = dz / d;

    // ── Outer radial wall (full circle keeps ball inside zone) ───────────────
    if (d > ro - BR) {
        pos.x = cx + nx * (ro - BR);
        pos.z = cz + nz * (ro - BR);
        float vdn = vel.x * nx + vel.z * nz;
        if (vdn > 0) { vel.x -= (1.f + r) * vdn * nx; vel.z -= (1.f + r) * vdn * nz; }
    }

    // ── Inner radial wall (only active in arc region, not in the SE gap) ─────
    // Gap = SE quadrant: dx > 0 AND dz < 0  (angle between -π/2 and 0)
    bool inGap = (dx > 0 && dz < 0);
    if (!inGap && d < ri + BR) {
        pos.x = cx + nx * (ri + BR);
        pos.z = cz + nz * (ri + BR);
        float vdn = vel.x * nx + vel.z * nz;
        if (vdn < 0) { vel.x -= (1.f + r) * vdn * nx; vel.z -= (1.f + r) * vdn * nz; }
    }

    // ── East hard cap: block going past outer wall on the east side ──────────
    if (pos.x > cx + ro - BR) {
        pos.x = cx + ro - BR;
        if (vel.x > 0) vel.x = -fabsf(vel.x) * r;
    }

    // ── South hard cap: block going past outer wall on the south side ────────
    if (pos.z < cz - ro + BR) {
        pos.z = cz - ro + BR;
        if (vel.z < 0) vel.z = fabsf(vel.z) * r;
    }

    // ── Channel bumper (cylindrical, west apex) ───────────────────────────────
    {
        float dbx = pos.x - H10_BUMP_X, dbz = pos.z - H10_BUMP_Z;
        float db2 = dbx * dbx + dbz * dbz;
        float minD = BR + H10_BUMP_R;
        if (db2 < minD * minD && db2 > 0.0001f) {
            float db  = sqrtf(db2);
            float nbx = dbx / db, nbz = dbz / db;
            float push = minD - db;
            pos.x += nbx * push;
            pos.z += nbz * push;
            float vdn = vel.x * nbx + vel.z * nbz;
            if (vdn < 0) { vel.x -= (1.f + r) * vdn * nbx; vel.z -= (1.f + r) * vdn * nbz; }
        }
    }
}

// ── nearCup ───────────────────────────────────────────────────────────────────
bool Hole10::nearCup(const vec3& pos) const {
    float dx = pos.x - H10_CUP_X;
    float dz = pos.z - H10_CUP_Z;
    return sqrtf(dx * dx + dz * dz) < 0.35f;
}

// ── getTeePos ─────────────────────────────────────────────────────────────────
vec3 Hole10::getTeePos() const {
    return { H10_TEE_X, 0.08f, H10_TEE_Z };
}
