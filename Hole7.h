#ifndef HOLE7_H
#define HOLE7_H

#include "Hole.h"

// ── Hole 7: S-shape (two connected semicircle arcs, travels east) ─────────────
static const float H7_Z0    = -46.5f;
static const float H7_X0    = -16.0f;
static const float H7_RA    =   5.0f;
static const float H7_LS    =   3.0f;
static const float H7_FW    =   4.0f;
static const float H7_WH7   =   0.45f;
static const float H7_WTH7  =   0.28f;

class Hole7 : public Hole {
    Mesh mFloor1, mFloor2;
    Mesh mWall1In, mWall1Out;
    Mesh mWall2In, mWall2Out;
public:
    Hole7(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    ~Hole7();

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
