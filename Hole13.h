#ifndef HOLE13_H
#define HOLE13_H

#include "Hole.h"

// ── Hole 14: Bowl with central depression ────────────────────────────────────
// Paraboloid bowl sunken into the terrain. Cup sits at the lowest point;
// gravity pulls the ball toward the centre. A small flat tee box leads in
// from the north so the ball can rest before the player hits it.
//
// Bowl surface: y(r) = -DEPTH * (1 - r²/R²)
// terrainForce returns slope-induced force pulling toward centre.

// Bowl: visible sand disk on top of grass + paraboloid mesh below for physics.
// Positioned roughly between zigzag (cup z=-5) and S-curve (tee z=6).
static const float H13_CX     = 32.0f;   // bowl centre x
static const float H13_CZ     =  2.0f;   // bowl centre z (shifted south, closer to H14)
static const float H13_R      =  3.0f;   // bowl outer radius
static const float H13_DEPTH  =  2.0f;   // depth at centre (deeper for visible slope)

// Flat tee box (more gap from zigzag, in the middle of the H12-H14 gap)
static const float H13_TXW    = 30.0f;
static const float H13_TXE    = 34.0f;
static const float H13_TZN    = -2.0f;
static const float H13_TZS    = -1.0f;

static const float H13_WH     =  0.40f;
static const float H13_WTH    =  0.25f;

class Hole13 : public Hole {
    Mesh mBowl;       // paraboloid bowl mesh, centred on origin
    Mesh mSandDisk;   // flat sand disk above grass — makes the bowl visible
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
