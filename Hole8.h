#ifndef HOLE8_H
#define HOLE8_H

#include "Hole.h"

// ── Hole 8: triangle, base west (entry), apex east (cup) ─────────────────────
static const float H8_BX   = 12.0f;   // base x  (west entry)
static const float H8_CZ   = -46.5f;  // centreline z  (same z as Hole 7)
static const float H8_HW   =  3.5f;   // half-width at base
static const float H8_LEN  =  9.0f;   // length base → apex
static const float H8_WH8  =  0.45f;
static const float H8_WTH8 =  0.28f;

class Hole8 : public Hole {
    Mesh mFloor;
public:
    Hole8(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    ~Hole8();

    void render(const mat4& vp) const override;
    void wallCollide(vec3& pos, vec3& vel) const override;
    bool nearCup(const vec3& pos) const override;
    vec3 getTeePos() const override;
};

#endif
