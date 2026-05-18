#ifndef CUBE_H
#define CUBE_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "Shape3D.h"

using namespace std;
using namespace glm;

class Cube : public Shape3D
{
private:
    float length;
    glm::vec3 centre;

public:
    Cube();
    Cube(glm::vec3 centre, float length);
    void initialise();
};

#endif