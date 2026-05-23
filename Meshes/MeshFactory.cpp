#include "MeshFactory.h"

MeshFactory::MeshFactory() {}

// Flat pentagon in XZ plane (triangle fan, circumradius 1, south vertex at +z)
Mesh MeshFactory::makePentagonFloor()
{
    vector<Vertex> V;
    vector<unsigned> I;
    V.push_back({{0, 0, 0}, {0, 1, 0}, {0.5f, 0.5f}});
    for (int k = 0; k <= 5; k++)
    {
        float t = PI / 2.0f + k * (2.0f * PI / 5.0f);
        float c = cosf(t), s = sinf(t);
        V.push_back({{c, 0, s}, {0, 1, 0}, {c * 0.5f + 0.5f, s * 0.5f + 0.5f}});
    }
    for (int k = 0; k < 5; k++)
        I.insert(I.end(), {0, (unsigned)(k + 1), (unsigned)(k + 2)});
    return m.upload(V, I);
}

// Flat circle in XZ plane (triangle fan, radius 1)
Mesh MeshFactory::makeCircle(int N)
{
    vector<Vertex> V;
    vector<unsigned> I;
    V.push_back({{0, 0, 0}, {0, 1, 0}, {0.5f, 0.5f}});
    for (int i = 0; i <= N; i++)
    {
        float t = 2 * PI * i / N;
        float c = cosf(t), s = sinf(t);
        V.push_back({{c, 0, s}, {0, 1, 0}, {c * 0.5f + 0.5f, s * 0.5f + 0.5f}});
    }
    for (int i = 0; i < N; i++)
        I.insert(I.end(), {0, (unsigned)(i + 1), (unsigned)(i + 2)});
    return m.upload(V, I);
}

// Curved arc wall: hollow annular strip from tStart to tEnd, radius R, thickness WTH, height WH
// Place at origin; translate to loop centre when drawing
Mesh MeshFactory::makeArcwall(float R, float WTH, float WH, float tStart, float tEnd, int N)
{
    vector<Vertex> V;
    vector<unsigned> I;
    float Ro = R + WTH * 0.5f, Ri = R - WTH * 0.5f;
    float dt = (tEnd - tStart) / N;
    for (int i = 0; i <= N; i++)
    {
        float t = tStart + i * dt, ct = cosf(t), st = sinf(t);
        vec3 no = {ct, 0, st}, ni = {-ct, 0, -st};
        V.push_back({{Ro * ct, 0, Ro * st}, no, {(float)i / N, 0}});
        V.push_back({{Ro * ct, WH, Ro * st}, no, {(float)i / N, 1}});
        V.push_back({{Ri * ct, 0, Ri * st}, ni, {(float)i / N, 0}});
        V.push_back({{Ri * ct, WH, Ri * st}, ni, {(float)i / N, 1}});
    }
    for (int i = 0; i < N; i++)
    {
        unsigned b = 4 * i;
        I.insert(I.end(), {b, b + 1, b + 4, b + 1, b + 5, b + 4});     // outer face
        I.insert(I.end(), {b + 2, b + 6, b + 3, b + 3, b + 6, b + 7}); // inner face
        I.insert(I.end(), {b + 1, b + 3, b + 5, b + 3, b + 7, b + 5}); // top cap
    }
    return m.upload(V, I);
}

// Filled arc strip in XZ plane (annular sector floor), facing up
Mesh MeshFactory::makeArcFloor(float Ri, float Ro, float tStart, float tEnd, int N)
{
    vector<Vertex> V;
    vector<unsigned> I;
    float dt = (tEnd - tStart) / N;
    for (int i = 0; i <= N; i++)
    {
        float t = tStart + i * dt, ct = cosf(t), st = sinf(t);
        float u = (float)i / N;
        V.push_back({{Ri * ct, 0, Ri * st}, {0, 1, 0}, {u, 0}});
        V.push_back({{Ro * ct, 0, Ro * st}, {0, 1, 0}, {u, 1}});
    }
    for (int i = 0; i < N; i++)
    {
        unsigned b = 2 * i;
        I.insert(I.end(), {b, b + 2, b + 1, b + 1, b + 2, b + 3});
    }
    return m.upload(V, I);
}

// Flat triangle in XZ plane, vertices given in local space, facing up
Mesh MeshFactory::makeTriFloor(vec3 a, vec3 b, vec3 c)
{
    vector<Vertex> V = {
        {a, {0, 1, 0}, {0.0f, 0.5f}},
        {b, {0, 1, 0}, {0.0f, 1.0f}},
        {c, {0, 1, 0}, {1.0f, 0.5f}},
    };
    return m.upload(V, {0, 1, 2});
}

// Horizontal quad in XZ plane, facing up, UV 0-1
Mesh MeshFactory::makeQuad() {
    vector<Vertex> V={
        {{-0.5f,0,-0.5f},{0,1,0},{0,0}},    // vertex,normal,texture
        {{ 0.5f,0,-0.5f},{0,1,0},{1,0}},
        {{ 0.5f,0, 0.5f},{0,1,0},{1,1}},
        {{-0.5f,0, 0.5f},{0,1,0},{0,1}},
    };
    return m.upload(V,{0,1,2,0,2,3}); // v and indices to draw
}

// Unit box [-0.5,0.5]^3
Mesh MeshFactory::makeBox() {
     vector<Vertex> V; vector<unsigned> I;
    struct F{ vec3 n,u,v,o; };  // faces : normal, right, up ,origin
    F fs[6]={
        {{0,0,1},{1,0,0},{0,1,0},{-0.5f,-0.5f,0.5f}},
        {{0,0,-1},{-1,0,0},{0,1,0},{0.5f,-0.5f,-0.5f}},
        {{1,0,0},{0,0,-1},{0,1,0},{0.5f,-0.5f,0.5f}},
        {{-1,0,0},{0,0,1},{0,1,0},{-0.5f,-0.5f,-0.5f}},
        {{0,1,0},{1,0,0},{0,0,-1},{-0.5f,0.5f,0.5f}},
        {{0,-1,0},{1,0,0},{0,0,1},{-0.5f,-0.5f,-0.5f}},
    };
    for(int f=0;f<6;f++){
        unsigned b=(unsigned)V.size();
        for(int vi=0;vi<4;vi++){
            float uu=(vi==1||vi==2)?1.f:0.f, vv=(vi==2||vi==3)?1.f:0.f;
            V.push_back({fs[f].o+fs[f].u*uu+fs[f].v*vv, fs[f].n, {uu,vv}});
        }
        I.insert(I.end(),{b,b+1,b+2,b,b+2,b+3});
    }
    return m.upload(V,I);
}

Mesh MeshFactory::makeSphere(int st, int sl) {}
Mesh MeshFactory::makeCylinder(int sl) {}
Mesh MeshFactory::makeTorus(float R, float r, int sl, int st) {}
Mesh MeshFactory::makeTrapezoid() {}
