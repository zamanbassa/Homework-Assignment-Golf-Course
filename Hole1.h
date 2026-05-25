#ifndef HOLE1_H
#define HOLE1_H

#include "Hole.h"

// Hole 1: straight fairway + bridge
static const float H1_CX   = -33.0f;
static const float H1_CZ   =  36.0f;
static const float H1_FW   =   4.0f;
static const float H1_FL   =  14.0f;
static const float H1_WH   =   0.45f;
static const float H1_WTH  =   0.30f;

class Hole1 : public Hole {
public:
    Hole1(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
