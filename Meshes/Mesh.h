#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

using namespace glm;
using namespace std;

class Mesh
{
private:
    GLuint vao, vbo, ebo;
    int count;

public:
    struct Vertex
    {
        vec3 pos, norm;
        vec2 uv;
    };

    Mesh();
    int Count() const;  // getCount()
    GLuint VAO() const; // getVAO
    GLuint VBO() const; // getVBO
    GLuint EBO() const; // getEBO
    Mesh upload(const vector<Vertex> &V, const vector<unsigned> &I);
    void freeMesh(Mesh &m);
};
#endif