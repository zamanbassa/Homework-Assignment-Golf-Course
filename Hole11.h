#ifndef HOLE11_H
#define HOLE11_H

#include "Hole.h"

// ── Hole 11: Long east-west straight just before the river ───────────────────
// Sits between hole 10's south extent (z=-29.5) and the river entry (z=-22).
// No obstacles — designated windmill placement zone. Player enters from west,
// plays east toward the bridge (hole 12).
//
//  z=-27  +-----------------------+   <- north wall
//         | tee           cup     |
//         |       (windmill)      |
//  z=-23  +-----------------------+   <- south wall (river just below)
//       x:22                    38

static const float H11_ZC   = -25.0f;  // fairway centre z
static const float H11_ZN   = -27.0f;  // north wall
static const float H11_ZS   = -23.0f;  // south wall (river entry just south at z=-22)
static const float H11_XW   =  22.0f;  // west wall (tee side)
static const float H11_XE   =  38.0f;  // east wall (cup side, near bridge)
static const float H11_WH   =  0.45f;
static const float H11_WTH  =  0.28f;

class Hole11 : public Hole {
public:
    Hole11(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
