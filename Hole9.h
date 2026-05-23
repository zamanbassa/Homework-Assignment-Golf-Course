#ifndef HOLE9_H
#define HOLE9_H

#include "Hole.h"

// ── Hole 9: U-shape fairway ───────────────────────────────────────────────────
// Two parallel N-S arms connected at their north ends by a connector.
// Ball enters west arm (tee at south opening), rolls north, rounds connector,
// returns south through east arm to the cup.
//
//  z=-53  +-------+     <- north wall (closed bottom of U)
//         |       |
//  z=-50.5| +-+-+ |     <- connector ends; inner bumpers at corners
//         | W|   | E|
//  z=-44  +--+   +--+   <- south openings (tee in west, cup in east)
//         x:23  27 29  33

static const float H9_WX1  = 23.0f;   // west arm outer-west  x
static const float H9_WX2  = 27.0f;   // west arm inner-east  x
static const float H9_EX1  = 29.0f;   // east arm inner-west  x
static const float H9_EX2  = 33.0f;   // east arm outer-east  x
static const float H9_ZN   = -44.0f;  // south (open) end of both arms
static const float H9_ZS   = -50.5f;  // north end of arms / start of connector
static const float H9_ZB   = -53.0f;  // north closed wall
static const float H9_WH   =  0.45f;  // wall height
static const float H9_WTH  =  0.28f;  // wall thickness
static const float H9_BUMP =  0.50f;  // corner-bumper cylinder radius

class Hole9 : public Hole {
public:
    Hole9(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
