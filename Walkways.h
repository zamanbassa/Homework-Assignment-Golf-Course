#ifndef WALKWAYS_H
#define WALKWAYS_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"

using namespace glm;

Mesh mQuad;

//defined in main
void draw(const Mesh& m, const mat4& model, const mat4& vp, int surf);

void drawWalkway(const mat4 &vp, float x1, float z1, float x2, float z2, float width = 2.0f);

#endif