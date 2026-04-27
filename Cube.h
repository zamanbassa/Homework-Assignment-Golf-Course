#ifndef CUBE_H
#define CUBE_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

class Cube
{
private:
    int length;
    glm::vec3 centre;

public:
    Cube();
    Cube(glm::vec3 centre);
    ~Cube();
};

#endif