#ifndef BALL_H
#define BALL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>
#include "BallGlobals.h"
#include "constants.h"

using namespace std;
using namespace glm;

class Ball
{

private:
    vec3 pos;
    vec3 vel;
    bool active;
    bool moving;
    bool inHole;
    int strokes; 


    public:
    Ball();
    void update(float dt);
    void wallCollide();
    void checkObstacles();
    bool nearCup();

    void setPos(vec3 p);
    void setVel(vec3 v);
    void setActive(bool a);
    void setMoving(bool m);
    void setInHole(bool ih);
    void setStrokes(int s);

    vec3 getPos() const;
    vec3 getVel() const;
    bool getActive() const;
    bool getMoving() const;
    bool getInHole() const;
    int getStrokes() const;

};

#endif