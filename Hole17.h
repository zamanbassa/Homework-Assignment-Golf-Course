#ifndef HOLE17_H
#define HOLE17_H

#include "Hole.h"
#include <vector>

// ── Hole 18: Final challenge (L-shape, mixed terrain, multi-obstacle, lights)
// A showcase hole that combines features from earlier holes: a long L-shape
// fairway, three terrain bands (grass → sand → concrete), bumpers, a stone
// column obstacle and lamp posts at each corner.
//
//   x: -8   -4                              14
// z=42  +---+--------------------------------+
//       |        EAST ARM B                  |  <- mixed terrain
// z=46  +---+----+----+----+----+----+----+--+
//           |
//           | A (north arm — grass tee)
//           |
//   z=49   +-+
//          tee at south end of A
//
// (Coordinates picked to sit south of the river path and south-west of the
//  rocks. River south of z=44 is around x=-11.)

// Segment A — tee arm (south arm, tee at south end, narrow). Smaller than before.
static const float H17_AXW =  22.0f;
static const float H17_AXE =  26.0f;
static const float H17_AZN =  39.0f;
static const float H17_AZS =  44.0f;

// Segment B — play arm (west arm, cup at west end). Smaller than before.
static const float H17_BXW =  14.0f;
static const float H17_BXE =  26.0f;
static const float H17_BZN =  39.0f;
static const float H17_BZS =  42.0f;

static const float H17_WH    = 0.45f;
static const float H17_WTH   = 0.28f;
static const float H17_BUMP  = 0.40f;
static const float H17_COL_R = 0.55f;   // central column (showcase obstacle)
static const float H17_COL_H = 2.2f;

class Hole17 : public Hole {
public:
    Hole17(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;

    // Lamp positions specific to this hole (for global lighting array).
    static const std::vector<glm::vec3>& lampPositions();
};

#endif
