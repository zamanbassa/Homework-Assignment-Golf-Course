#include "Hole.h"
#include <glm/gtc/matrix_transform.hpp>

Hole::Hole(int n, const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : holeNum(n), mQuad(q), mBox(b), mCylinder(c), mTorus(t) {}

int Hole::getNumber() const {
    return holeNum;
}

void Hole::drawCup(const mat4& vp, float cx, float cz) const {
    const float HALF_PI = 1.5707963f;
    mat4 m;

    m = glm::translate(mat4(1), {cx, 0.01f, cz});
    m = glm::scale(m, {0.35f, 0.1f, 0.35f});
    draw(*mTorus, m, vp, 19);

    m = glm::translate(mat4(1), {cx, 0.001f, cz});
    m = glm::scale(m, {0.33f, 1.f, 0.33f});
    draw(*mQuad, m, vp, 19);

    m = glm::translate(mat4(1), {cx + 0.38f, 0.f, cz});
    m = glm::scale(m, {0.05f, 2.2f, 0.05f});
    draw(*mCylinder, m, vp, 16);

    m = glm::translate(mat4(1), {cx + 0.655f, 2.0f, cz});
    m = glm::rotate(m, HALF_PI, {1, 0, 0});
    m = glm::scale(m, {0.55f, 1.0f, 0.32f});
    draw(*mQuad, m, vp, 17);
}

void Hole::drawWall(const mat4& vp, float cx, float cy, float cz,
                    float sx, float sy, float sz, float yRot) const {
    mat4 m = glm::translate(mat4(1), {cx, cy, cz});
    if (yRot != 0.f)
        m = glm::rotate(m, yRot, {0, 1, 0});
    m = glm::scale(m, {sx, sy, sz});
    draw(*mBox, m, vp, 8);
}
