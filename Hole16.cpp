#include "Hole16.h"
#include <cmath>
#include <vector>

static const float PI16 = 3.14159265358979f;

static Mesh h16_makeDisk(float radius, float y, int segs = 48) {
    std::vector<Vertex>   V;
    std::vector<unsigned> I;
    V.push_back({{0.f, y, 0.f}, {0, 1, 0}, {0.5f, 0.5f}});
    for (int i = 0; i < segs; i++) {
        float th = 2.f * PI16 * i / segs;
        float cx = radius * cosf(th);
        float cz = radius * sinf(th);
        float u  = 0.5f + 0.5f * cosf(th);
        float v  = 0.5f + 0.5f * sinf(th);
        V.push_back({{cx, y, cz}, {0, 1, 0}, {u, v}});
    }
    for (int i = 0; i < segs; i++) {
        unsigned a = 0;
        unsigned b = 1 + i;
        unsigned c = 1 + (i + 1) % segs;
        I.insert(I.end(), {a, c, b});
    }
    return upload(V, I);
}

static Mesh h16_makeAnnulus(float rIn, float rOut, float y, int segs = 48) {
    std::vector<Vertex>   V;
    std::vector<unsigned> I;
    for (int i = 0; i < segs; i++) {
        float th = 2.f * PI16 * i / segs;
        float ct = cosf(th), st = sinf(th);
        V.push_back({{rIn  * ct, y, rIn  * st}, {0, 1, 0}, {(float)i / segs, 0.f}});
        V.push_back({{rOut * ct, y, rOut * st}, {0, 1, 0}, {(float)i / segs, 1.f}});
    }
    for (int i = 0; i < segs; i++) {
        unsigned a = 2 * i;
        unsigned b = 2 * i + 1;
        unsigned c = 2 * ((i + 1) % segs);
        unsigned d = 2 * ((i + 1) % segs) + 1;
        I.insert(I.end(), {a, c, b, b, c, d});
    }
    return upload(V, I);
}

Hole16::Hole16(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(16, q, b, c, t)
{
    mIsland = h16_makeDisk(H16_ISLAND_R, H16_ISLAND_Y);
    mWater  = h16_makeAnnulus(H16_ISLAND_R - 0.05f, H16_WATER_R, 0.003f);
}

Hole16::~Hole16() {
    glDeleteBuffers(1, &mIsland.vbo); glDeleteBuffers(1, &mIsland.ebo); glDeleteVertexArrays(1, &mIsland.vao);
    glDeleteBuffers(1, &mWater.vbo);  glDeleteBuffers(1, &mWater.ebo);  glDeleteVertexArrays(1, &mWater.vao);
}

void Hole16::render(const mat4& vp) const {
    { mat4 m = glm::translate(mat4(1), {H16_CX, 0.f, H16_CZ});
      draw(mWater, m, vp, 5); }

    { mat4 m = glm::translate(mat4(1), {H16_CX, 0.f, H16_CZ});
      draw(mIsland, m, vp, 0); }

    // tee
    { float midX = (H16_TXW + H16_TXE) * 0.5f;
      float midZ = (H16_TZN + H16_TZS) * 0.5f;
      mat4 m = glm::translate(mat4(1), {midX, 0.f, midZ});
      m = glm::scale(m, {H16_TXE - H16_TXW, 1.f, H16_TZS - H16_TZN});
      draw(*mQuad, m, vp, 0);
      // marker
      mat4 t = glm::translate(mat4(1), {midX, 0.005f, midZ});
      t = glm::scale(t, {(H16_TXE - H16_TXW) - 0.4f, 1.f, (H16_TZS - H16_TZN) - 0.3f});
      draw(*mQuad, t, vp, 1);
    }

    drawWall(vp, (H16_TXW + H16_TXE) * 0.5f, H16_WH * 0.5f, H16_TZN - H16_WTH * 0.5f,
             (H16_TXE - H16_TXW) + H16_WTH * 2.f, H16_WH, H16_WTH);
    drawWall(vp, H16_TXW - H16_WTH * 0.5f, H16_WH * 0.5f, (H16_TZN + H16_TZS) * 0.5f,
             H16_WTH, H16_WH, (H16_TZS - H16_TZN) + H16_WTH * 2.f);
    drawWall(vp, H16_TXE + H16_WTH * 0.5f, H16_WH * 0.5f, (H16_TZN + H16_TZS) * 0.5f,
             H16_WTH, H16_WH, (H16_TZS - H16_TZN) + H16_WTH * 2.f);

    {
        const int RIM_SEGS = 60;
        const float rimR   = H16_ISLAND_R + 0.03f;
        const float segW   = 2.f * PI16 * rimR / RIM_SEGS;
        for (int i = 0; i < RIM_SEGS; i++) {
            float th = 2.f * PI16 * (i + 0.5f) / RIM_SEGS;
            if (sinf(th) < -0.55f) continue;
            float dx = cosf(th) * rimR;
            float dz = sinf(th) * rimR;
            mat4 m = glm::translate(mat4(1), {H16_CX + dx, H16_RIM_H * 0.5f + H16_ISLAND_Y, H16_CZ + dz});
            m = glm::rotate(m, th + 1.5707963f, {0, 1, 0});
            m = glm::scale(m, {0.10f, H16_RIM_H, segW * 1.08f});
            draw(*mBox, m, vp, 10);
        }
    }

    {
        const float HALF_PI = 1.5707963f;
        const float cy = H16_ISLAND_Y + 0.01f;
        { mat4 m = glm::translate(mat4(1), {H16_CX, cy, H16_CZ});
          m = glm::scale(m, {0.35f, 0.1f, 0.35f}); draw(*mTorus, m, vp, 19); }
        { mat4 m = glm::translate(mat4(1), {H16_CX, cy - 0.005f, H16_CZ});
          m = glm::scale(m, {0.33f, 1.f, 0.33f}); draw(*mQuad, m, vp, 19); }
        { mat4 m = glm::translate(mat4(1), {H16_CX + 0.38f, cy - 0.01f, H16_CZ});
          m = glm::scale(m, {0.05f, 2.2f, 0.05f}); draw(*mCylinder, m, vp, 16); }
        { mat4 m = glm::translate(mat4(1), {H16_CX + 0.655f, cy + 2.0f, H16_CZ});
          m = glm::rotate(m, HALF_PI, {1, 0, 0});
          m = glm::scale(m, {0.55f, 1.0f, 0.32f}); draw(*mQuad, m, vp, 17); }
    }
}

void Hole16::wallCollide(vec3& pos, vec3& vel) const {
    const float r  = 0.72f;
    const float BR = 0.08f;

    if (pos.z < H16_TZS + 0.5f && pos.z > H16_TZN - 1.f
        && pos.x > H16_TXW - 1.f && pos.x < H16_TXE + 1.f) {
        if (pos.z - BR < H16_TZN) {
            pos.z = H16_TZN + BR; if (vel.z < 0) vel.z = fabsf(vel.z) * r;
        }
        if (pos.x - BR < H16_TXW && pos.z < H16_TZS) {
            pos.x = H16_TXW + BR; if (vel.x < 0) vel.x = fabsf(vel.x) * r;
        }
        if (pos.x + BR > H16_TXE && pos.z < H16_TZS) {
            pos.x = H16_TXE - BR; if (vel.x > 0) vel.x = -fabsf(vel.x) * r;
        }
    }

    float dx = pos.x - H16_CX;
    float dz = pos.z - H16_CZ;
    float d  = sqrtf(dx * dx + dz * dz);
    bool inNorthGap = (dz < 0.f && fabsf(dx) < H16_ISLAND_R * 0.45f);
    if (d > H16_ISLAND_R && d < H16_ISLAND_R + 0.4f && !inNorthGap) {
        float nx = dx / d, nz = dz / d;
        pos.x = H16_CX + nx * (H16_ISLAND_R - BR);
        pos.z = H16_CZ + nz * (H16_ISLAND_R - BR);
        float vdn = vel.x * nx + vel.z * nz;
        if (vdn > 0) { vel.x -= (1.f + r) * vdn * nx; vel.z -= (1.f + r) * vdn * nz; }
    }
}

bool Hole16::nearCup(const vec3& pos) const {
    float dx = pos.x - H16_CX;
    float dz = pos.z - H16_CZ;
    return sqrtf(dx * dx + dz * dz) < 0.40f;
}

vec3 Hole16::getTeePos() const {
    return { (H16_TXW + H16_TXE) * 0.5f, 0.08f, (H16_TZN + H16_TZS) * 0.5f };
}

float Hole16::groundY(const vec3& pos) const {
    float dx = pos.x - H16_CX;
    float dz = pos.z - H16_CZ;
    float d  = sqrtf(dx * dx + dz * dz);
    if (d <= H16_ISLAND_R) return H16_ISLAND_Y + BALL_R_CONST;
    return BALL_R_CONST;
}

vec3 Hole16::terrainForce(const vec3& pos) const {
    float dx = pos.x - H16_CX;
    float dz = pos.z - H16_CZ;
    float d2 = dx * dx + dz * dz;
    if (d2 < 0.0001f || d2 > H16_ISLAND_R * H16_ISLAND_R) return vec3(0.f);
    float d  = sqrtf(d2);
    float mag = H16_INCLINE * 4.0f;
    return vec3(-mag * dx / d, 0.f, -mag * dz / d);
}
