#ifndef HOLE14_H
#define HOLE14_H

#include "Hole.h"
#include "Mesh.h"
#include <vector>

// ── Hole 15: Smooth double-curve fairway on grass ─────────────────────────────
// Centreline polyline traces an S-shape from the tee (NE) to the cup (SW).
// Floor strip + side walls follow the curve smoothly. Decorative palm trees
// are placed by main.cpp at the positions returned by treePositions().

static const float H14_HW       =  1.60f;  // fairway half-width (smaller)
static const float H14_WH       =  0.45f;
static const float H14_WTH      =  0.22f;
static const float H14_TEE_X    = 32.0f;   // close to bowl cup at (32, 1)
static const float H14_TEE_Z    =  6.0f;
static const float H14_CUP_X    = 25.0f;
static const float H14_CUP_Z    = 14.0f;

class Hole14 : public Hole {
    Mesh mFloor;
    Mesh mWallL;
    Mesh mWallR;
    std::vector<glm::vec3> mCenter;   // centreline points (cached for collision)
public:
    Hole14(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    ~Hole14();

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;

    // Tree positions on the outside of the curves (called by main.cpp).
    static const std::vector<glm::vec3>& treePositions();
};

#endif
