#ifndef HOLE10_H
#define HOLE10_H

#include "Hole.h"

// Hole 10: 270 arc
static const float H10_CX     = 34.0f;
static const float H10_CZ     = -35.0f;
static const float H10_RI     =  2.5f;
static const float H10_RO     =  5.5f;
static const float H10_WH     =  0.45f;
static const float H10_WTH    =  0.28f;
static const float H10_ROCK_R =  1.2f;
static const float H10_BUMP_R =  0.50f;

class Hole10 : public Hole {
    Mesh mArcFloor;
    Mesh mArcWallIn;
    Mesh mArcWallOut;
public:
    Hole10(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    ~Hole10();

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
