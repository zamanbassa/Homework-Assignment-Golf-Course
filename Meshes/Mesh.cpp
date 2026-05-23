#include "Mesh.h"

Mesh::Mesh() {}

int Mesh::Count() const
{
    return count;
}

GLuint Mesh::VAO() const
{
    return vao;
}

GLuint Mesh::VBO() const
{
    return vbo;
}

GLuint Mesh::EBO() const
{
    return ebo;
}

void Mesh::setVAO(GLuint vao)
{
    this->vao = vao;
}

void Mesh::setVBO(GLuint vbo)
{
    this->vbo = vbo;
}

void Mesh::setEBO(GLuint ebo)
{
    this->ebo = ebo;
}

void Mesh::setCount(int c)
{
    this->count = count;
}

Mesh Mesh::upload(const vector<Vertex> &V, const vector<unsigned> &I)
{
    // create empty mesh
    Mesh m;
    m.count = (int)I.size();

    // create vao and buffers
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);
    glBindVertexArray(m.vao);

    // bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, V.size() * sizeof(Vertex), V.data(), GL_STATIC_DRAW);

    // bind ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, I.size() * sizeof(unsigned), I.data(), GL_STATIC_DRAW);

    // attributes for shaders
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, norm));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    return m;
}

void Mesh::freeMesh()
{
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}