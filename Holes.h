#ifndef HOLES_H
#define HOLES_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Meshes/Mesh.h"
#include "constants.h"
#include "BallGlobals.h"

using namespace glm;

extern GLuint gProg, skyProg;
extern Mesh mQuad, mBox, mSphere, mCylinder, mTorus, mTrap, mSkybox, mCircle;
extern Mesh mH3Wall1, mH3Wall2;                                   // hole 3 arc walls (south loop, north loop)
extern Mesh mH4Floor, mH4WallIn, mH4WallOut;                      // hole 4 banana meshes
extern Mesh mH5Floor;                                             // hole 5 pentagon floor
extern Mesh mH7FloorArc1, mH7FloorArc2;                           // hole 7 S-shape floors
extern Mesh mH7WallA1In, mH7WallA1Out, mH7WallA2In, mH7WallA2Out; // hole 7 S-shape walls
extern Mesh mH8Floor;                                             // hole 8 triangle floor

void draw(const Mesh &m, const mat4 &model, const mat4 &vp, int surf);
void drawHole(const mat4 &vp, float cx, float cz);
void drawHole2(const mat4 &vp, float cx, float cz);
void drawHole3(const mat4 &vp, float cx, float cz);
void drawHole4(const mat4 &vp, float cx, float cz);
void drawHole5(const mat4 &vp, float cx, float cz);
void drawHole6(const mat4 &vp, float cx, float cz);
void drawHole7(const mat4 &vp, float cx, float cz);
void drawHole8(const mat4 &vp, float cx, float cz);

#endif