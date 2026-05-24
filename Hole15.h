#ifndef HOLE15_H
#define HOLE15_H

#include "Hole.h"

// ── Hole 16: Narrow corridor with column obstacles ──────────────────────────
// Long, narrow NS corridor. Four cylindrical columns rise along the centreline,
// forcing the ball to weave around them. Flat surface — precision required.
//
//    x: 30.5  33.5
// z=24    +---+    <- north wall (tee end)
//         |tee|
//         | C1|
//         |   |
//         | C2|
//         |   |
//         | C3|
//         |   |
//         | C4|
//         |cup|
// z=40    +---+    <- south wall

// Positioned in the grass between the pond (ends at x≈24) and the SE-corner
// rocks (start at x≈33). Shorter than before (12 long instead of 15).
static const float H15_CX     = 27.0f;   // corridor centre x
static const float H15_FW     =  3.0f;   // narrow width (x = 25.5..28.5)
static const float H15_XW     = H15_CX - H15_FW * 0.5f;   // 25.5
static const float H15_XE     = H15_CX + H15_FW * 0.5f;   // 28.5
static const float H15_ZN     = 18.0f;   // north wall (tee end) — closer to S-curve
static const float H15_ZS     = 30.0f;   // south wall (cup end)
static const float H15_WH     =  0.45f;
static const float H15_WTH    =  0.28f;
static const float H15_COL_R  =  0.35f;  // column cylinder radius
static const float H15_COL_H  =  1.6f;   // column visible height (taller than walls)

class Hole15 : public Hole {
public:
    Hole15(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
