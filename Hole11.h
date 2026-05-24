#ifndef HOLE11_H
#define HOLE11_H

#include "Hole.h"

// ── Hole 11: East-west straight, on the ground ────────────────────────────────
// Long EW fairway, tee at west end, cup at east end.
// Ground-level (no elevation). Shifted west towards the hills.
//
//  z=-27  +============================+   <- north wall
//         | tee                   cup  |
//  z=-23  +============================+   <- south wall
//        x:14                         34

static const float H11_ZC        = -25.0f;   // fairway centre z
static const float H11_ZN        = -27.0f;   // north wall
static const float H11_ZS        = -23.0f;   // south wall
static const float H11_XW        =  14.0f;   // west wall (tee side)
static const float H11_XE        =  34.0f;   // east wall (cup side)
static const float H11_WH        =   0.45f;  // wall height
static const float H11_WTH       =   0.28f;  // wall thickness

class Hole11 : public Hole {
public:
    Hole11(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
