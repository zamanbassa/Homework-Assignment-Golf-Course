#ifndef HOLE16_H
#define HOLE16_H

#include "Hole.h"

// Hole 16: island green
static const float H16_CX        = 22.0f;
static const float H16_CZ        = 35.0f;
static const float H16_ISLAND_R  =  1.8f;
static const float H16_WATER_R   =  3.0f;
static const float H16_ISLAND_Y  =  0.05f;
static const float H16_RIM_H     =  0.45f;
static const float H16_INCLINE   =  0.08f;
static const float H16_WH        =  0.40f;
static const float H16_WTH       =  0.22f;

// tee strip
static const float H16_TXW       = 20.0f;
static const float H16_TXE       = 24.0f;
static const float H16_TZN       = 30.5f;
static const float H16_TZS       = 31.5f;

class Hole16 : public Hole {
    Mesh mIsland;
    Mesh mWater;
public:
    Hole16(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    ~Hole16();

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;

    float groundY(const vec3& pos) const override;
    vec3  terrainForce(const vec3& pos) const override;
};

#endif
