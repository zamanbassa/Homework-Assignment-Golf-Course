#include "Hole14.h"
#include <cmath>

using glm::vec3;

// ─── Centreline polyline (smooth double curve from tee to cup) ────────────────
static std::vector<vec3> h14_buildCenterline() {
    return {
        {32.0f, 0.f,  6.0f},   // tee (close to bowl cup at 32,1)
        {32.0f, 0.f,  7.5f},
        {31.5f, 0.f,  9.0f},   // first bend — curving south-west
        {30.5f, 0.f, 10.2f},
        {29.5f, 0.f, 11.0f},   // mid — slight inflection
        {28.5f, 0.f, 11.8f},
        {27.5f, 0.f, 12.6f},   // second bend — curving west again
        {26.0f, 0.f, 13.4f},
        {25.0f, 0.f, 14.0f},   // cup
    };
}

// ─── Floor strip mesh: flat triangles between left and right edge points ─────
static Mesh h14_makeFloor(const std::vector<vec3>& pts, float hw) {
    std::vector<Vertex>   V;
    std::vector<unsigned> I;
    int n = (int)pts.size();
    for (int i = 0; i < n; i++) {
        vec3 dir;
        if      (i == 0)     dir = glm::normalize(pts[1]   - pts[0]);
        else if (i == n - 1) dir = glm::normalize(pts[n-1] - pts[n-2]);
        else                 dir = glm::normalize(pts[i+1] - pts[i-1]);
        vec3 perp = {-dir.z, 0.f, dir.x};
        float u   = (float)i / (n - 1);
        vec3 L = {pts[i].x - perp.x * hw, 0.f, pts[i].z - perp.z * hw};
        vec3 R = {pts[i].x + perp.x * hw, 0.f, pts[i].z + perp.z * hw};
        V.push_back({L, {0, 1, 0}, {u, 0.f}});
        V.push_back({R, {0, 1, 0}, {u, 1.f}});
    }
    for (int i = 0; i < n - 1; i++) {
        unsigned b = 2 * i;
        I.insert(I.end(), {b, b + 2, b + 1, b + 1, b + 2, b + 3});
    }
    return upload(V, I);
}

// ─── Wall strip mesh: vertical wall following the strip edge ─────────────────
// side = -1 for left, +1 for right. WallThickness extrudes outward.
static Mesh h14_makeWall(const std::vector<vec3>& pts, float hw, float wth,
                          float wallH, int side) {
    std::vector<Vertex>   V;
    std::vector<unsigned> I;
    int n = (int)pts.size();
    float inner = hw * (float)side;
    float outer = (hw + wth) * (float)side;

    for (int i = 0; i < n; i++) {
        vec3 dir;
        if      (i == 0)     dir = glm::normalize(pts[1]   - pts[0]);
        else if (i == n - 1) dir = glm::normalize(pts[n-1] - pts[n-2]);
        else                 dir = glm::normalize(pts[i+1] - pts[i-1]);
        vec3 perp = {-dir.z, 0.f, dir.x};   // points to +right side
        float u   = (float)i / (n - 1);

        vec3 innerBase = {pts[i].x + perp.x * inner, 0.f,    pts[i].z + perp.z * inner};
        vec3 outerBase = {pts[i].x + perp.x * outer, 0.f,    pts[i].z + perp.z * outer};
        vec3 innerTop  = {innerBase.x,               wallH, innerBase.z};
        vec3 outerTop  = {outerBase.x,               wallH, outerBase.z};

        // Inward-facing normal (toward fairway centre)
        vec3 nIn = {-perp.x * (float)side, 0.f, -perp.z * (float)side};

        V.push_back({innerBase, nIn, {u, 0.f}});  // 0 inner base
        V.push_back({innerTop,  nIn, {u, 1.f}});  // 1 inner top
        V.push_back({outerBase, -nIn, {u, 0.f}}); // 2 outer base
        V.push_back({outerTop,  -nIn, {u, 1.f}}); // 3 outer top
    }
    for (int i = 0; i < n - 1; i++) {
        unsigned a = 4 * i;
        // Inner face (4 verts: a, a+1, a+4, a+5)
        if (side > 0) {
            I.insert(I.end(), {a,     a + 4, a + 1, a + 1, a + 4, a + 5});
            I.insert(I.end(), {a + 2, a + 3, a + 6, a + 3, a + 7, a + 6});
            I.insert(I.end(), {a + 1, a + 5, a + 3, a + 3, a + 5, a + 7});
        } else {
            I.insert(I.end(), {a,     a + 1, a + 4, a + 1, a + 5, a + 4});
            I.insert(I.end(), {a + 2, a + 6, a + 3, a + 3, a + 6, a + 7});
            I.insert(I.end(), {a + 1, a + 3, a + 5, a + 3, a + 7, a + 5});
        }
    }
    return upload(V, I);
}

Hole14::Hole14(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(14, q, b, c, t)
{
    mCenter = h14_buildCenterline();
    mFloor  = h14_makeFloor(mCenter, H14_HW);
    mWallL  = h14_makeWall(mCenter, H14_HW, H14_WTH, H14_WH, -1);
    mWallR  = h14_makeWall(mCenter, H14_HW, H14_WTH, H14_WH, +1);
}

Hole14::~Hole14() {
    glDeleteBuffers(1, &mFloor.vbo); glDeleteBuffers(1, &mFloor.ebo); glDeleteVertexArrays(1, &mFloor.vao);
    glDeleteBuffers(1, &mWallL.vbo); glDeleteBuffers(1, &mWallL.ebo); glDeleteVertexArrays(1, &mWallL.vao);
    glDeleteBuffers(1, &mWallR.vbo); glDeleteBuffers(1, &mWallR.ebo); glDeleteVertexArrays(1, &mWallR.vao);
}

void Hole14::render(const mat4& vp) const {
    // Floor (grass)
    draw(mFloor, mat4(1), vp, 0);

    // Tee box at the tee end (oriented along initial direction)
    {
        vec3 dir = glm::normalize(mCenter[1] - mCenter[0]);
        float ang = atan2f(dir.x, dir.z);
        mat4 m = glm::translate(mat4(1), {H14_TEE_X, 0.005f, H14_TEE_Z});
        m = glm::rotate(m, ang, {0, 1, 0});
        m = glm::scale(m, {H14_HW * 2.f - 0.2f, 1.f, 1.2f});
        draw(*mQuad, m, vp, 1);
    }

    // Side walls
    draw(mWallL, mat4(1), vp, 8);
    draw(mWallR, mat4(1), vp, 8);

    // End caps (tee + cup ends) — short box walls perpendicular to centreline
    auto endCap = [&](const vec3& p, const vec3& dir) {
        float ang = atan2f(dir.x, dir.z);
        mat4 m = glm::translate(mat4(1), {p.x, H14_WH * 0.5f, p.z});
        m = glm::rotate(m, ang, {0, 1, 0});
        m = glm::scale(m, {(H14_HW + H14_WTH) * 2.f, H14_WH, H14_WTH});
        draw(*mBox, m, vp, 8);
    };
    endCap(mCenter.front(), glm::normalize(mCenter[1] - mCenter[0]));
    endCap(mCenter.back(),  glm::normalize(mCenter.back() - mCenter[mCenter.size() - 2]));

    // Cup
    drawCup(vp, H14_CUP_X, H14_CUP_Z);
}

// Distance-from-centreline collision: for each segment, find closest point and
// push the ball back inside the corridor if it has crossed an edge.
void Hole14::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f;
    const int n = (int)mCenter.size();
    if (n < 2) return;

    // Find the closest centreline segment
    int   bestSeg = 0;
    float bestD2  = 1e9f;
    float bestT   = 0.f;
    for (int i = 0; i < n - 1; i++) {
        vec3 a = mCenter[i], b = mCenter[i + 1];
        vec3 ab = b - a;
        float ab2 = ab.x * ab.x + ab.z * ab.z;
        if (ab2 < 1e-6f) continue;
        float t = ((pos.x - a.x) * ab.x + (pos.z - a.z) * ab.z) / ab2;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        float cx = a.x + ab.x * t, cz = a.z + ab.z * t;
        float dx = pos.x - cx, dz = pos.z - cz;
        float d2 = dx * dx + dz * dz;
        if (d2 < bestD2) { bestD2 = d2; bestSeg = i; bestT = t; }
    }

    // Compute perpendicular distance and signed side relative to segment direction
    vec3 a   = mCenter[bestSeg];
    vec3 b   = mCenter[bestSeg + 1];
    vec3 dir = glm::normalize(b - a);
    vec3 perp = {-dir.z, 0.f, dir.x};
    vec3 cp   = a + dir * (bestT * glm::length(b - a));
    float dx  = pos.x - cp.x, dz = pos.z - cp.z;
    float side = dx * perp.x + dz * perp.z;

    float limit = H14_HW - BR;
    if (fabsf(side) > limit) {
        float over = (fabsf(side) - limit);
        float s    = (side > 0.f) ? 1.f : -1.f;
        pos.x -= s * over * perp.x;
        pos.z -= s * over * perp.z;
        float vdn = vel.x * perp.x + vel.z * perp.z;
        if ((s > 0.f && vdn > 0.f) || (s < 0.f && vdn < 0.f)) {
            vel.x -= (1.f + r) * vdn * perp.x;
            vel.z -= (1.f + r) * vdn * perp.z;
        }
    }

    // End caps — only when ball is past first or last segment
    if (bestSeg == 0 && bestT <= 0.f) {
        vec3 d0 = glm::normalize(mCenter[1] - mCenter[0]);
        float along = (pos.x - mCenter[0].x) * d0.x + (pos.z - mCenter[0].z) * d0.z;
        if (along < BR) {
            float push = BR - along;
            pos.x += d0.x * push; pos.z += d0.z * push;
            float vdn = vel.x * d0.x + vel.z * d0.z;
            if (vdn < 0.f) { vel.x -= (1.f + r) * vdn * d0.x; vel.z -= (1.f + r) * vdn * d0.z; }
        }
    }
    if (bestSeg == n - 2 && bestT >= 1.f) {
        vec3 dN = glm::normalize(mCenter[n-1] - mCenter[n-2]);
        float along = (pos.x - mCenter[n-1].x) * dN.x + (pos.z - mCenter[n-1].z) * dN.z;
        if (along > -BR) {
            float push = along + BR;
            pos.x -= dN.x * push; pos.z -= dN.z * push;
            float vdn = vel.x * dN.x + vel.z * dN.z;
            if (vdn > 0.f) { vel.x -= (1.f + r) * vdn * dN.x; vel.z -= (1.f + r) * vdn * dN.z; }
        }
    }
}

bool Hole14::nearCup(const vec3& pos) const {
    float dx = pos.x - H14_CUP_X;
    float dz = pos.z - H14_CUP_Z;
    return sqrtf(dx * dx + dz * dz) < 0.35f;
}

vec3 Hole14::getTeePos() const {
    return { H14_TEE_X, 0.08f, H14_TEE_Z };
}

// Tree positions on the outside of each curve — picked by hand alongside the strip.
const std::vector<vec3>& Hole14::treePositions() {
    static const std::vector<vec3> pts = {
        {33.6f, 0.f,  5.5f},   // beyond tee NE
        {29.5f, 0.f,  8.8f},   // outside first bend (west)
        {33.0f, 0.f, 10.0f},   // outside first bend (east)
        {26.8f, 0.f, 11.2f},
        {30.5f, 0.f, 12.0f},
        {23.5f, 0.f, 14.5f},   // beyond cup SW
        {26.5f, 0.f, 15.0f},
    };
    return pts;
}
