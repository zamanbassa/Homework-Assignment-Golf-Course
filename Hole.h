#ifndef HOLE_H
#define HOLE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Mesh.h"

using glm::vec3;
using glm::mat4;

// Defined in main.cpp (not static so Hole translation units can call it)
void draw(const Mesh& m, const mat4& model, const mat4& vp, int surf);

static const float BALL_R_CONST = 0.08f;  // ball radius, used by all holes

class Hole {
protected:
    int         holeNum;
    const Mesh* mQuad;
    const Mesh* mBox;
    const Mesh* mCylinder;
    const Mesh* mTorus;

    void drawCup(const mat4& vp, float cx, float cz) const;
    void drawWall(const mat4& vp, float cx, float cy, float cz,
                  float sx, float sy, float sz, float yRot = 0.f) const;

public:
    Hole(int n, const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t);
    virtual ~Hole() {}

    // Must be implemented by each hole
    virtual void render(const mat4& vp) const = 0;
    virtual void wallCollide(vec3& pos, vec3& vel) const = 0;
    virtual bool nearCup(const vec3& pos) const = 0;
    virtual vec3 getTeePos() const = 0;

    // Optional terrain effects (override for hills, ramps, etc.)
    // Returns world-Y of ball centre. Default = flat ground (BALL_R_CONST).
    virtual float groundY(const vec3& pos) const;
    // Returns additional velocity push per second from terrain slopes.
    virtual vec3  terrainForce(const vec3& pos) const;

    int getNumber() const;
};

#endif
