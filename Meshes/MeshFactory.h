#ifndef MESHFACTORY_H
#define MESHFACTORY_H

#include "Mesh.h"
#include "Vertex.h"
#include "../constants.h"

class MeshFactory
{
private:
    Mesh m;

public:
    MeshFactory();
    Mesh makePentagonFloor();
    Mesh makeCircle(int N=32);
    Mesh makeArcWall(float R, float WTH, float WH, float tStart, float tEnd, int N=48);
    Mesh makeArcFloor(float Ri, float Ro, float tStart, float tEnd, int N=48);
    Mesh makeTriFloor(vec3 a, vec3 b, vec3 c);
    Mesh makeQuad();
    Mesh makeBox();
    Mesh makeSphere(int st = 14, int sl = 22);
    Mesh makeCylinder(int sl = 18);
    Mesh makeTorus(float R = 1.f, float r = 0.05f, int sl = 28, int st = 10);
    Mesh makeTrapezoid();
    Mesh makeSkyboxMesh();
};

#endif