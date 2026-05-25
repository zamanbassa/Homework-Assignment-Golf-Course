#ifndef HOLE11_H
#define HOLE11_H

#include "Hole.h"

// Hole 11: EW straight
static const float H11_ZC        = -25.0f;
static const float H11_ZN        = -27.0f;
static const float H11_ZS        = -23.0f;
static const float H11_XW        =  14.0f;
static const float H11_XE        =  34.0f;
static const float H11_WH        =   0.45f;
static const float H11_WTH       =   0.28f;

class Hole11 : public Hole {
public:
    Hole11(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
