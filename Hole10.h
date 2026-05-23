#ifndef HOLE10_H
#define HOLE10_H

#include "Hole.h"

// ── Hole 10 layout constants ──────────────────────────────────────────────────
// 3/4-circle (270°) arc fairway, counterclockwise from east (tee) to north (cup).
// A central rock pillar decorates the inside; a channel bumper sits at the west
// apex (angle=π) to make the ball work around it.
//
//  Arc center:  (H10_CX, 0, H10_CZ)
//  Inner radius H10_RI, outer radius H10_RO  → channel width = 3.0
//  Arc:  t = 0 (east/tee) → t = 3π/2 (north/cup), counterclockwise
//  Gap (no walls):  NE quadrant, t ∈ (3π/2, 2π)

static const float H10_CX     = 34.0f;   // arc centre x
static const float H10_CZ     = -35.0f;  // arc centre z
static const float H10_RI     =  2.5f;   // inner fairway radius
static const float H10_RO     =  5.5f;   // outer fairway radius
static const float H10_WH     =  0.45f;  // wall height
static const float H10_WTH    =  0.28f;  // wall thickness
static const float H10_ROCK_R =  1.2f;   // central decorative rock radius
static const float H10_BUMP_R =  0.50f;  // channel bumper (west apex) radius

class Hole10 : public Hole {
    Mesh mArcFloor;
    Mesh mArcWallIn;
    Mesh mArcWallOut;
public:
    Hole10(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    ~Hole10();

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
