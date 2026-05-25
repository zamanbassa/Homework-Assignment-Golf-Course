#ifndef HOLE5_H
#define HOLE5_H

#include "Hole.h"

// Hole 5: pentagon
static const float H5_CX   = -33.0f;
static const float H5_CZ   = -31.0f;
static const float H5_R    =   6.0f;
static const float H5_WH5  =   0.45f;
static const float H5_WTH5 =   0.30f;

class Hole5 : public Hole {
    Mesh mFloor;
public:
    Hole5(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    ~Hole5();

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
