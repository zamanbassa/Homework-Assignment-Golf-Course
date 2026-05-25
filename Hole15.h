#ifndef HOLE15_H
#define HOLE15_H

#include "Hole.h"

// Hole 15: column corridor
static const float H15_CX     = 27.0f;
static const float H15_FW     =  3.0f;
static const float H15_XW     = H15_CX - H15_FW * 0.5f;
static const float H15_XE     = H15_CX + H15_FW * 0.5f;
static const float H15_ZN     = 18.0f;
static const float H15_ZS     = 30.0f;
static const float H15_WH     =  0.45f;
static const float H15_WTH    =  0.28f;
static const float H15_COL_R  =  0.35f;
static const float H15_COL_H  =  1.6f;

class Hole15 : public Hole {
public:
    Hole15(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
