#ifndef HOLE18_H
#define HOLE18_H

#include "Hole.h"
#include <vector>

// ── Hole 19: Lighthouse Plaza (the creative final hole) ──────────────────────
// A celebratory closing hole. A square stone-walled plaza encloses a tall
// central lighthouse tower with a glowing lantern at the top. The cup sits
// at the south-east corner of the plaza; the ball must navigate AROUND the
// lighthouse to reach it. Decorative palm trees and lamp posts complete the
// scene.
//
//   x: -33                                -19
// z=33  +------------------------------+   <- north wall (tee end)
//       |       tee                    |
//       |      [Lighthouse tower]      |
//       |             X                |
//       |          (cup)               |
// z=46  +------------------------------+   <- south wall

static const float H18_XW    = 12.0f;
static const float H18_XE    = 20.0f;
static const float H18_ZN    = 45.0f;
static const float H18_ZS    = 53.0f;

static const float H18_WH    =  0.50f;
static const float H18_WTH   =  0.30f;

// Central lighthouse tower — compact to fit the square plaza (8×8)
static const float H18_LH_X  = 16.0f;
static const float H18_LH_Z  = 49.0f;
static const float H18_LH_R  =  0.75f;   // tower base radius
static const float H18_LH_H  =  4.0f;    // tower body height
static const float H18_LANT_R = 0.95f;   // glowing lantern radius
static const float H18_LANT_H = 0.75f;   // lantern height
static const float H18_ROOF_H = 0.55f;   // cap roof height

class Hole18 : public Hole {
public:
    Hole18(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;

    // Plaza decoration positions used by main.cpp.
    static const std::vector<glm::vec3>& lampPositions();
    static const std::vector<glm::vec3>& treePositions();
};

#endif
