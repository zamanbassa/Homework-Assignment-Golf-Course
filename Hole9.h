#ifndef HOLE9_H
#define HOLE9_H

#include "Hole.h"

// ── Hole 9 layout constants (all in world units) ─────────────────────────────
// U-shape: two parallel N-S arms connected at their south ends by a connector.
// Ball enters west arm (tee), rolls south, rounds the connector, returns north
// through east arm to the cup.
//
//  z=-28  +--+   +--+   <- north openings (tee in west, cup in east)
//         | W|   | E|
//  z=-40  | +-+-+ |     <- arms end; inner bumpers at corners
//         |       |
//  z=-43  +-------+     <- south wall
//         x:23  27 29  33

static const float H9_WX1  = 23.0f;   // west arm outer-west  x
static const float H9_WX2  = 27.0f;   // west arm inner-east  x  (= divider left)
static const float H9_EX1  = 29.0f;   // east arm inner-west  x  (= divider right)
static const float H9_EX2  = 33.0f;   // east arm outer-east  x
static const float H9_ZN   = -28.0f;  // north end of both arms
static const float H9_ZS   = -40.0f;  // south end of arms / start of connector
static const float H9_ZB   = -43.0f;  // bottom (south outer wall)
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