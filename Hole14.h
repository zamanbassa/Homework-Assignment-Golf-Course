#ifndef HOLE14_H
#define HOLE14_H

#include "Hole.h"
#include "Mesh.h"
#include <vector>

// Hole 14: S-curve
static const float H14_HW       =  1.60f;
static const float H14_WH       =  0.45f;
static const float H14_WTH      =  0.22f;
static const float H14_TEE_X    = 32.0f;
static const float H14_TEE_Z    =  6.0f;
static const float H14_CUP_X    = 25.0f;
static const float H14_CUP_Z    = 14.0f;

class Hole14 : public Hole {
    Mesh mFloor;
    Mesh mWallL;
    Mesh mWallR;
    std::vector<glm::vec3> mCenter;
public:
    Hole14(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    ~Hole14();

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;

    static const std::vector<glm::vec3>& treePositions();
};

#endif
