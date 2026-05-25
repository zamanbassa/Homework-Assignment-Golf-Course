#ifndef HOLE6_H
#define HOLE6_H

#include "Hole.h"

// Hole 6: L-shape
static const float H6_CX     = -33.0f;
static const float H6_TEE_Z  = -38.0f;
static const float H6_S1_W   =   4.0f;
static const float H6_S1_L   =   6.5f;
static const float H6_S2_W   =   4.0f;
static const float H6_S2_L   =  16.0f;
static const float H6_WH6    =   0.45f;
static const float H6_WTH6   =   0.30f;

class Hole6 : public Hole {
public:
    Hole6(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
