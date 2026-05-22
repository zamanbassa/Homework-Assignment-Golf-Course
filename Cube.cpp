#include "Cube.h"

Cube::Cube() : centre({0.0f, 0.0f, 0.0f}), length(0) {}

Cube::Cube(vec3 centre, float length) : centre(centre), length(length) {}

void Cube::initialise()
{
    // clear vectors
    vertices.clear();
    normals.clear();
    indices.clear();

    
    float half_length = length / 2.0f;
    float x, y, z;

    // face 1 - front face
    x = centre.x;
    y = centre.y;
    z = centre.z - half_length;

    // face 2 - back face
    x = centre.x;
    y = centre.y;
    z = centre.z + half_length;

    // face 3 - right face
    x = centre.x + half_length;
    y = centre.y;
    z = centre.z;

    // face 4 - l
}

/// helper function to make faces
vector<float> makeFace(float x, float y, float z)
{
}
