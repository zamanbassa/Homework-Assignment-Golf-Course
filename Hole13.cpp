#include "Hole13.h"
#include <cmath>
#include <vector>

static const float PI13 = 3.14159265358979f;

static Mesh h13_makeSandDisk(float R, int segs = 48) {
    std::vector<Vertex>   V;
    std::vector<unsigned> I;
    V.push_back({{0.f, 0.f, 0.f}, {0, 1, 0}, {0.5f, 0.5f}});
    for (int i = 0; i < segs; i++) {
        float th = 2.f * PI13 * i / segs;
        float cx = R * cosf(th);
        float cz = R * sinf(th);
        float u  = 0.5f + 0.5f * cosf(th);
        float v  = 0.5f + 0.5f * sinf(th);
        V.push_back({{cx, 0.f, cz}, {0, 1, 0}, {u, v}});
    }
    for (int i = 0; i < segs; i++) {
        I.insert(I.end(), {(unsigned)0, (unsigned)(1 + (i + 1) % segs), (unsigned)(1 + i)});
    }
    return upload(V, I);
}

static Mesh h13_makeBowl(float R, float depth, int rings = 14, int segs = 32) {
    std::vector<Vertex>   V;
    std::vector<unsigned> I;

    V.push_back({{0.f, -depth, 0.f}, {0.f, 1.f, 0.f}, {0.5f, 0.5f}});

    for (int i = 1; i <= rings; i++) {
        float r = R * (float)i / rings;
        float y = -depth * (1.0f - (r * r) / (R * R));
        for (int j = 0; j < segs; j++) {
            float th = 2.f * PI13 * j / segs;
            float x  = r * cosf(th);
            float z  = r * sinf(th);
            glm::vec3 n = glm::normalize(glm::vec3(
                -2.f * depth * x / (R * R),
                 1.f,
                -2.f * depth * z / (R * R)
            ));
            float u = 0.5f + 0.5f * (x / R);
            float v = 0.5f + 0.5f * (z / R);
            V.push_back({{x, y, z}, n, {u, v}});
        }
    }

    for (int j = 0; j < segs; j++) {
        unsigned a = 0;
        unsigned b = 1 + j;
        unsigned c = 1 + (j + 1) % segs;
        I.insert(I.end(), {a, c, b});  // CCW from above
    }

    for (int i = 1; i < rings; i++) {
        unsigned b0 = 1 + (i - 1) * segs;
        unsigned b1 = 1 + i * segs;
        for (int j = 0; j < segs; j++) {
            unsigned a = b0 + j;
            unsigned b = b0 + (j + 1) % segs;
            unsigned c = b1 + j;
            unsigned d = b1 + (j + 1) % segs;
            I.insert(I.end(), {a, d, b, a, c, d});  // CCW from above
        }
    }

    return upload(V, I);
}

Hole13::Hole13(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(13, q, b, c, t)
{
    mBowl     = h13_makeBowl(H13_R, H13_DEPTH);
    mSandDisk = h13_makeSandDisk(H13_R);
}

Hole13::~Hole13() {
    glDeleteBuffers(1, &mBowl.vbo);
    glDeleteBuffers(1, &mBowl.ebo);
    glDeleteVertexArrays(1, &mBowl.vao);
    glDeleteBuffers(1, &mSandDisk.vbo);
    glDeleteBuffers(1, &mSandDisk.ebo);
    glDeleteVertexArrays(1, &mSandDisk.vao);
}

void Hole13::render(const mat4& vp) const {
    { mat4 m = glm::translate(mat4(1), {H13_CX, 0.006f, H13_CZ});
      draw(mSandDisk, m, vp, 4); }

    { mat4 m = glm::translate(mat4(1), {H13_CX, 0.005f, H13_CZ});
      draw(mBowl, m, vp, 4); }

    // tee
    { float midX = (H13_TXW + H13_TXE) * 0.5f;
      float midZ = (H13_TZN + H13_TZS) * 0.5f;
      mat4 m = glm::translate(mat4(1), {midX, 0.f, midZ});
      m = glm::scale(m, {H13_TXE - H13_TXW, 1.f, H13_TZS - H13_TZN});
      draw(*mQuad, m, vp, 0); }

    { float midZ = (H13_TZN + H13_TZS) * 0.5f;
      mat4 m = glm::translate(mat4(1), {(H13_TXW + H13_TXE) * 0.5f, 0.005f, midZ});
      m = glm::scale(m, {(H13_TXE - H13_TXW) - 0.4f, 1.f, 1.0f});
      draw(*mQuad, m, vp, 1); }

    drawWall(vp, (H13_TXW + H13_TXE) * 0.5f, H13_WH * 0.5f, H13_TZN - H13_WTH * 0.5f,
             (H13_TXE - H13_TXW) + H13_WTH * 2.f, H13_WH, H13_WTH);
    drawWall(vp, H13_TXW - H13_WTH * 0.5f, H13_WH * 0.5f, (H13_TZN + H13_TZS) * 0.5f,
             H13_WTH, H13_WH, (H13_TZS - H13_TZN) + H13_WTH * 2.f);
    drawWall(vp, H13_TXE + H13_WTH * 0.5f, H13_WH * 0.5f, (H13_TZN + H13_TZS) * 0.5f,
             H13_WTH, H13_WH, (H13_TZS - H13_TZN) + H13_WTH * 2.f);

    const float cupX  = H13_CX;
    const float cupZ  = H13_CZ;
    const float cupY  = -H13_DEPTH + 0.01f;
    const float HALF_PI = 1.5707963f;

    { mat4 m = glm::translate(mat4(1), {cupX, cupY, cupZ});
      m = glm::scale(m, {0.35f, 0.1f, 0.35f});
      draw(*mTorus, m, vp, 19); }
    { mat4 m = glm::translate(mat4(1), {cupX, cupY - 0.005f, cupZ});
      m = glm::scale(m, {0.33f, 1.f, 0.33f});
      draw(*mQuad, m, vp, 19); }

    { mat4 m = glm::translate(mat4(1), {cupX, 0.008f, cupZ});
      m = glm::scale(m, {0.45f, 1.f, 0.45f});
      draw(*mQuad, m, vp, 19); }
    { mat4 m = glm::translate(mat4(1), {cupX, 0.010f, cupZ});
      m = glm::scale(m, {0.50f, 0.06f, 0.50f});
      draw(*mTorus, m, vp, 19); }
    { mat4 m = glm::translate(mat4(1), {cupX + 0.38f, cupY - 0.01f, cupZ});
      m = glm::scale(m, {0.05f, 5.5f, 0.05f});
      draw(*mCylinder, m, vp, 16); }
    { mat4 m = glm::translate(mat4(1), {cupX + 0.655f, cupY + 5.0f, cupZ});
      m = glm::rotate(m, HALF_PI, {1, 0, 0});
      m = glm::scale(m, {0.55f, 1.0f, 0.32f});
      draw(*mQuad, m, vp, 17); }
}

void Hole13::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f;

    if (pos.z < H13_TZS && pos.z > H13_TZN - 1.f) {
        if (pos.x - BR < H13_TXW && pos.x > H13_TXW - 1.f) {
            pos.x = H13_TXW + BR; if (vel.x < 0) vel.x = fabsf(vel.x) * r;
        }
        if (pos.x + BR > H13_TXE && pos.x < H13_TXE + 1.f) {
            pos.x = H13_TXE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r;
        }
        if (pos.z - BR < H13_TZN) {
            pos.z = H13_TZN + BR; if (vel.z < 0) vel.z = fabsf(vel.z) * r;
        }
    }

    float dx = pos.x - H13_CX, dz = pos.z - H13_CZ;
    float d2 = dx * dx + dz * dz;
    const float RIM = H13_R + 0.6f;
    if (d2 > RIM * RIM) {
        bool inTeeCorridor = (pos.x > H13_TXW - BR && pos.x < H13_TXE + BR
                              && pos.z < H13_TZS + 0.5f);
        if (!inTeeCorridor) {
            float d = sqrtf(d2);
            float nx = dx / d, nz = dz / d;
            pos.x = H13_CX + nx * (RIM - BR);
            pos.z = H13_CZ + nz * (RIM - BR);
            float vdn = vel.x * nx + vel.z * nz;
            if (vdn > 0) { vel.x -= (1.f + r) * vdn * nx; vel.z -= (1.f + r) * vdn * nz; }
        }
    }
}

bool Hole13::nearCup(const vec3& pos) const {
    float dx = pos.x - H13_CX;
    float dz = pos.z - H13_CZ;
    return sqrtf(dx * dx + dz * dz) < 0.45f;
}

vec3 Hole13::getTeePos() const {
    return { (H13_TXW + H13_TXE) * 0.5f, 0.08f, (H13_TZN + H13_TZS) * 0.5f };
}

float Hole13::groundY(const vec3& pos) const {
    float dx = pos.x - H13_CX;
    float dz = pos.z - H13_CZ;
    float r2 = dx * dx + dz * dz;
    if (r2 >= H13_R * H13_R) return BALL_R_CONST;
    float yBowl = -H13_DEPTH * (1.0f - r2 / (H13_R * H13_R));
    return yBowl + BALL_R_CONST;
}

vec3 Hole13::terrainForce(const vec3& pos) const {
    float dx = pos.x - H13_CX;
    float dz = pos.z - H13_CZ;
    float r2 = dx * dx + dz * dz;
    if (r2 < 0.0001f || r2 >= H13_R * H13_R) return vec3(0.f);
    float r = sqrtf(r2);
    const float g = 12.f;
    float slope = 2.f * H13_DEPTH * r / (H13_R * H13_R);
    float mag = g * slope;
    return vec3(-mag * dx / r, 0.f, -mag * dz / r);
}
