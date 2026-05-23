#ifndef VERTEX_H
#define VERTEX_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace glm;

struct Vertex
{
    vec3 pos, norm;
    vec2 uv;
};

#endif
