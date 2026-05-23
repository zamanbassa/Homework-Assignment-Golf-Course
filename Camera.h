#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

using namespace std;
using namespace glm;

class Camera
{

private:
    vec3 pos;
    float yaw;
    float pitch;
    float fov;
    float spd;
    bool ortho;

public:
    Camera();
    vec3 fwd() const;
    mat4 view() const;
    mat4 proj(float a) const;

    void setPos(vec3 p);
    void setYaw(float y);
    void setPitch(float p);
    void setFOV(float f);
    void setSpd(float s);
    void setOrtho(bool o);

    vec3 getPos() const;
    float getYaw() const;
    float getPitch() const;
    float getFOV() const;
    float getSpd() const;
    bool getOrtho() const;
};

#endif