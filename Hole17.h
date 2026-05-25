#ifndef HOLE17_H
#define HOLE17_H

#include "Hole.h"
#include <vector>

// Hole 17: L-shape final
// Segment A
static const float H17_AXW =  22.0f;
static const float H17_AXE =  26.0f;
static const float H17_AZN =  39.0f;
static const float H17_AZS =  44.0f;

// Segment B
static const float H17_BXW =  14.0f;
static const float H17_BXE =  26.0f;
static const float H17_BZN =  39.0f;
static const float H17_BZS =  42.0f;

static const float H17_WH    = 0.45f;
static const float H17_WTH   = 0.28f;
static const float H17_BUMP  = 0.40f;
static const float H17_COL_R = 0.55f;
static const float H17_COL_H = 2.2f;

class Hole17 : public Hole {
public:
    Hole17(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;

    static const std::vector<glm::vec3>& lampPositions();
};

#endif
