#ifndef SHAPE3D_H
#define SHAPE3D_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

using namespace std;

class Shape3D
{
protected:
    vector<float> vertices;
    vector<float> normals;
    vector<unsigned int> indices;
    vector<float> colours;
    vector<float> texture_coords;

    glm::vec4 colour;

public:
    Shape3D();
    Shape3D(glm::vec4 colour);
    vector<float> getVertices() const;
    vector<unsigned int> getIndices() const;
    vector<float> getColours() const;
    vector<float> getNormals() const;
    vector<float> getTextureCoords() const;
    void setVertices(vector<float> vertices);
    void setNormals(vector<float> normals);
    void setIndices(vector<unsigned int> indices);
    void setTextureCoords(vector<float> texture);
    void setColour(glm::vec4 colour);
};

#endif