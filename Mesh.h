#ifndef MESH_H
#define MESH_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

using glm::vec2;
using glm::vec3;

struct Vertex { vec3 pos, norm; vec2 uv; };
struct Mesh   { GLuint vao, vbo, ebo; int count; };

// defined in main.cpp (not static so Hole classes can call it)
Mesh upload(const std::vector<Vertex>& V, const std::vector<unsigned>& I);

#endif