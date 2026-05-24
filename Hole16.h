#ifndef HOLE16_H
#define HOLE16_H

#include "Hole.h"

// ── Hole 17: Island green ────────────────────────────────────────────────────
// A small circular green raised slightly above a surrounding water ring.
// Cup sits at the centre of a gently inclined island. The player tees off
// from a grass strip north of the water and must land the ball on the island.
//
//                north tee strip
//                  +---tee---+
//                  |         |
//   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~     water radius
//   ~~~~~~~~~~        +-----+        ~~~~~~~~~~
//   ~~~~~~~~          |cup  |          ~~~~~~~~     island (R = 3.0)
//   ~~~~~~~~~~        +-----+        ~~~~~~~~~~
//   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const float H16_CX        = 22.0f;   // island centre x
static const float H16_CZ        = 35.0f;   // island centre z (south of H15 corridor)
static const float H16_ISLAND_R  =  1.8f;
static const float H16_WATER_R   =  3.0f;   // outer water radius
static const float H16_ISLAND_Y  =  0.05f;  // raised slightly above water
static const float H16_RIM_H     =  0.45f;  // taller rim — looks like a rounded wall
static const float H16_INCLINE   =  0.08f;  // ground slope coefficient
static const float H16_WH        =  0.40f;
static const float H16_WTH       =  0.22f;

// Tee strip — grass strip just south of H15 cup (at z=28)
static const float H16_TXW       = 20.0f;
static const float H16_TXE       = 24.0f;
static const float H16_TZN       = 30.5f;
static const float H16_TZS       = 31.5f;

class Hole16 : public Hole {
    Mesh mIsland;   // raised disk mesh
    Mesh mWater;    // surrounding water annulus mesh
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
