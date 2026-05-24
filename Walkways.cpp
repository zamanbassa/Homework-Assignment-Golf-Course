#include "Walkways.h"

void drawWalkway(const mat4 &vp, float x1, float z1, float x2, float z2, float width = 2.0f)
{
    float dx = x2 - x1;
    float dz = z2 - z1;
    float len = sqrtf(dx*dx + dz*dz);
    float angle = atan2f(dx, dz);
    float cx = (x1 + x2) * 0.5f;
    float cz = (z1 + z2) * 0.5f;

    mat4 m = glm::translate(mat4(1), {cx, 0.002f, cz});
    m = glm::rotate(m, angle, {0, 1, 0});
    m = glm::scale(m, {width, 1, len});
    draw(mQuad, m, vp, 3);
}