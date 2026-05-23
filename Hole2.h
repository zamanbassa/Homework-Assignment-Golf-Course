#ifndef HOLE2_H
#define HOLE2_H

#include "Hole.h"

// ── Hole 2: straight fairway with hill obstacle ───────────────────────────────
static const float H2_CX   = -33.0f;
static const float H2_CZ   =  18.0f;
static const float H2_FW   =   4.0f;
static const float H2_FL   =  14.0f;
static const float H2_WH   =   0.45f;
static const float H2_WTH  =   0.30f;
static const float H2_HILL_R =  1.5f;   // hill dome radius
static const float H2_HILL_H =  0.42f;  // hill dome height

class Hole2 : public Hole {
    const Mesh* mSphere;
public:
    Hole2(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t, const Mesh* sphere);

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;

    // Hill terrain: raises ball over the dome and pushes it down the slope
    float groundY(const vec3& pos) const override;
    vec3  terrainForce(const vec3& pos) const override;
};

#endif
