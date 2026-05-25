#ifndef HOLE9_H
#define HOLE9_H

#include "Hole.h"

// Hole 9: U-shape
static const float H9_WX1  = 23.0f;
static const float H9_WX2  = 27.0f;
static const float H9_EX1  = 29.0f;
static const float H9_EX2  = 33.0f;
static const float H9_ZN   = -44.0f;
static const float H9_ZS   = -50.5f;
static const float H9_ZB   = -53.0f;
static const float H9_WH   =  0.45f;
static const float H9_WTH  =  0.28f;
static const float H9_BUMP =  0.50f;

class Hole9 : public Hole {
public:
    Hole9(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
