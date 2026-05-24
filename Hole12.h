#ifndef HOLE12_H
#define HOLE12_H

#include "Hole.h"

// ── Hole 13: Z-shape zigzag on concrete ──────────────────────────────────────
// Three connected rectangular segments form a Z (top arm → middle connector →
// bottom arm). Concrete floor for fast roll. Side barriers are the inner-corner
// walls; the path itself enforces the zigzag.
//
//   x: 34   36                  40
// z=-12  +-+-+------------------+   <- north wall of top arm
//        |       TOP ARM (A)    |
// z=-10  +-+-+------------------+   <- south of A east of B
//        | B |
//        | B |  CONNECTOR
//        | B |
// z=-6   +-+-+--+--+--+--+--+   <- north of C west of B
//        |       BOTTOM ARM (C)
// z=-4   +---+--+--+--+--+--+
//        30                 36

// Segment A — top arm (EW, tee at east end)
static const float H12_AXW   = 34.0f;
static const float H12_AXE   = 40.0f;
static const float H12_AZN   = -12.0f;
static const float H12_AZS   = -10.0f;

// Segment B — middle connector (NS)
static const float H12_BXW   = 34.0f;
static const float H12_BXE   = 36.0f;
static const float H12_BZN   = -10.0f;
static const float H12_BZS   =  -6.0f;

// Segment C — bottom arm (EW, cup at west end)
static const float H12_CXW   = 30.0f;
static const float H12_CXE   = 36.0f;
static const float H12_CZN   =  -6.0f;
static const float H12_CZS   =  -4.0f;

static const float H12_WH    =  0.45f;
static const float H12_WTH   =  0.28f;

class Hole12 : public Hole {
public:
    Hole12(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
