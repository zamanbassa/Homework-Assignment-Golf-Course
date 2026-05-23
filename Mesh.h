#ifndef MESH_H
#define MESH_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

using glm::vec2;
using glm::vec3;

struct Vertex { vec3 pos, norm; vec2 uv; };
struct Mesh   { GLuint vao, vbo, ebo; int count; };

// ── All defined in main.cpp (not static so Hole classes can call them) ────────
Mesh upload(const std::vector<Vertex>& V, const std::vector<unsigned>& I);

// Primitive builders
Mesh makeQuad();
Mesh makeBox();
Mesh makeSphere(int stacks = 14, int slices = 22);
Mesh makeCylinder(int slices = 18);
Mesh makeTorus(float R = 1.f, float r = 0.05f, int sl = 28, int st = 10);
Mesh makeCircle(int N = 32);

// Specialised shape builders (used by Hole classes)
Mesh makePentagonFloor();
Mesh makeTriFloor(vec3 a, vec3 b, vec3 c);
Mesh makeArcFloor(float Ri, float Ro, float tStart, float tEnd, int N = 48);
Mesh makeArcWall(float R, float WTH, float WH, float tStart, float tEnd, int N = 48);

#endif
