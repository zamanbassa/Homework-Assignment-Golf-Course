#ifndef HOLE13_H
#define HOLE13_H

#include "Hole.h"

// Hole 13: bowl
static const float H13_CX     = 32.0f;
static const float H13_CZ     =  2.0f;
static const float H13_R      =  3.0f;
static const float H13_DEPTH  =  2.0f;

// tee box
static const float H13_TXW    = 30.0f;
static const float H13_TXE    = 34.0f;
static const float H13_TZN    = -2.0f;
static const float H13_TZS    = -1.0f;

static const float H13_WH     =  0.40f;
static const float H13_WTH    =  0.25f;

class Hole13 : public Hole {
    Mesh mBowl;
    Mesh mSandDisk;
public:
    Hole13(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    ~Hole13();

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;

    float groundY(const vec3& pos) const override;
    vec3  terrainForce(const vec3& pos) const override;
};

#endif
