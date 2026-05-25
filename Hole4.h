#ifndef HOLE4_H
#define HOLE4_H

#include "Hole.h"

// Hole 4: banana arc
static const float H4_CX   = -37.0f;
static const float H4_CZ   = -11.0f;
static const float H4_RI   =   7.0f;
static const float H4_RO   =  11.0f;
static const float H4_T0   =  -1.41f;
static const float H4_T1   =   0.0f;
static const float H4_WH4  =   0.45f;
static const float H4_WTH4 =   0.28f;

class Hole4 : public Hole {
    Mesh mFloor;
    Mesh mWallIn;
    Mesh mWallOut;
public:
    Hole4(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    ~Hole4();

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
