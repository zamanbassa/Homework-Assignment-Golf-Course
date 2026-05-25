#ifndef HOLE3_H
#define HOLE3_H

#include "Hole.h"

// Hole 3: figure-8
static const float H3_CX    = -33.0f;
static const float H3_CZ    =  -1.0f;
static const float H3_R     =   3.5f;
static const float H3_OBSR  =   0.75f;
static const float H3_WTH3  =   0.28f;
static const float H3_WH3   =   0.45f;
static const float H3_GAP   =   0.55f;

class Hole3 : public Hole {
    const Mesh* mCircle;
    Mesh mWall1;
    Mesh mWall2;
public:
    Hole3(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t, const Mesh* circle);
    ~Hole3();

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
