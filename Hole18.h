#ifndef HOLE18_H
#define HOLE18_H

#include "Hole.h"
#include <vector>

// Hole 18: lighthouse plaza
static const float H18_XW    = 12.0f;
static const float H18_XE    = 20.0f;
static const float H18_ZN    = 45.0f;
static const float H18_ZS    = 53.0f;

static const float H18_WH    =  0.50f;
static const float H18_WTH   =  0.30f;

// lighthouse tower
static const float H18_LH_X  = 16.0f;
static const float H18_LH_Z  = 49.0f;
static const float H18_LH_R  =  0.75f;
static const float H18_LH_H  =  4.0f;
static const float H18_LANT_R = 0.95f;
static const float H18_LANT_H = 0.75f;
static const float H18_ROOF_H = 0.55f;

class Hole18 : public Hole {
public:
    Hole18(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;

    static const std::vector<glm::vec3>& lampPositions();
    static const std::vector<glm::vec3>& treePositions();
};

#endif
