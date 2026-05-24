#ifndef HOLE12_H
#define HOLE12_H

#include "Hole.h"

// ── Hole 12: Narrow bridge crossing the river ────────────────────────────────
// Narrow NS bridge spans the river entry segment around z=-19. Wooden floor,
// taller railings, water hazard on either side. Precision shot required.
// North end flush with hole 11's south wall at z=-23.
//
//      x: 37   39
//  z=-23  +---+   <- north end (flush with H11)
//         |tee|
//         | | |   <- bridge over river (river crosses at z~-19)
//         |cup|
//  z=-13  +---+   <- south end (just south of river bank)

static const float H12_CX    = 38.0f;   // bridge centre x
static const float H12_FW    =  2.0f;   // narrow width (x=37..39)
static const float H12_XW    = H12_CX - H12_FW * 0.5f;  // 37.0
static const float H12_XE    = H12_CX + H12_FW * 0.5f;  // 39.0
static const float H12_ZN    = -23.0f;  // north wall (flush with H11 south)
static const float H12_ZS    = -13.0f;  // south wall (south of river bank)
static const float H12_WH    =  0.60f;  // taller wall = railing
static const float H12_WTH   =  0.18f;  // thinner railing
static const float H12_POST_R = 0.18f;  // bridge post radius
static const float H12_FLOOR_Y = 0.012f; // bridge floor slightly above river (river at 0.003)

class Hole12 : public Hole {
public:
    Hole12(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
