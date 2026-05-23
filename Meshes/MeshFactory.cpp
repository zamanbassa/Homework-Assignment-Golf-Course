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
Mesh MeshFactory::makeArcWall(float R, float WTH, float WH, float tStart, float tEnd, int N)
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
Mesh MeshFactory::makeQuad()
{
    vector<Vertex> V = {
        {{-0.5f, 0, -0.5f}, {0, 1, 0}, {0, 0}}, // vertex,normal,texture
        {{0.5f, 0, -0.5f}, {0, 1, 0}, {1, 0}},
        {{0.5f, 0, 0.5f}, {0, 1, 0}, {1, 1}},
        {{-0.5f, 0, 0.5f}, {0, 1, 0}, {0, 1}},
    };
    return m.upload(V, {0, 1, 2, 0, 2, 3}); // v and indices to draw
}

// Unit box [-0.5,0.5]^3
Mesh MeshFactory::makeBox()
{
    vector<Vertex> V;
    vector<unsigned> I;
    struct F
    {
        vec3 n, u, v, o;
    }; // faces : normal, right, up ,origin
    F fs[6] = {
        {{0, 0, 1}, {1, 0, 0}, {0, 1, 0}, {-0.5f, -0.5f, 0.5f}},
        {{0, 0, -1}, {-1, 0, 0}, {0, 1, 0}, {0.5f, -0.5f, -0.5f}},
        {{1, 0, 0}, {0, 0, -1}, {0, 1, 0}, {0.5f, -0.5f, 0.5f}},
        {{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {-0.5f, -0.5f, -0.5f}},
        {{0, 1, 0}, {1, 0, 0}, {0, 0, -1}, {-0.5f, 0.5f, 0.5f}},
        {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}, {-0.5f, -0.5f, -0.5f}},
    };
    for (int f = 0; f < 6; f++)
    {
        unsigned b = (unsigned)V.size();
        for (int vi = 0; vi < 4; vi++)
        {
            float uu = (vi == 1 || vi == 2) ? 1.f : 0.f, vv = (vi == 2 || vi == 3) ? 1.f : 0.f;
            V.push_back({fs[f].o + fs[f].u * uu + fs[f].v * vv, fs[f].n, {uu, vv}});
        }
        I.insert(I.end(), {b, b + 1, b + 2, b, b + 2, b + 3});
    }
    return m.upload(V, I);
}

// Sphere (for ball)
Mesh MeshFactory::makeSphere(int st, int sl)
{
    vector<Vertex> V;
    vector<unsigned> I;
    for (int i = 0; i <= st; i++)
    {
        float phi = PI * i / st;
        for (int j = 0; j <= sl; j++)
        {
            float th = 2 * PI * j / sl;
            vec3 n = {sinf(phi) * cosf(th), cosf(phi), sinf(phi) * sinf(th)};
            V.push_back({n, n, {(float)j / sl, (float)i / st}});
        }
    }
    for (int i = 0; i < st; i++)
        for (int j = 0; j < sl; j++)
        {
            unsigned a = i * (sl + 1) + j, b = a + 1, c = (i + 1) * (sl + 1) + j, d = c + 1;
            I.insert(I.end(), {a, c, b, b, c, d});
        }
    return m.upload(V, I);
}

// Cylinder (for flag pole & restaurant pillars)
Mesh MeshFactory::makeCylinder(int sl)
{
    vector<Vertex> V;
    vector<unsigned> I;
    for (int i = 0; i <= sl; i++)
    {
        float th = 2 * PI * i / sl;
        vec3 n = {cosf(th), 0, sinf(th)};
        V.push_back({{0.5f * cosf(th), 0, 0.5f * sinf(th)}, n, {(float)i / sl, 0}});
        V.push_back({{0.5f * cosf(th), 1, 0.5f * sinf(th)}, n, {(float)i / sl, 1}});
    }
    for (int i = 0; i < sl; i++)
    {
        unsigned a = 2 * i, b = 2 * i + 1, c = 2 * (i + 1), d = 2 * (i + 1) + 1;
        I.insert(I.end(), {a, b, c, b, d, c});
    }
    // top cap
    unsigned tc = (unsigned)V.size();
    V.push_back({{0, 1, 0}, {0, 1, 0}, {0.5f, 0.5f}});
    for (int i = 0; i <= sl; i++)
    {
        float th = 2 * PI * i / sl;
        V.push_back({{0.5f * cosf(th), 1, 0.5f * sinf(th)}, {0, 1, 0}, {0.5f + 0.5f * cosf(th), 0.5f + 0.5f * sinf(th)}});
    }
    for (int i = 0; i < sl; i++)
        I.insert(I.end(), {tc, tc + 1 + i, tc + 2 + i});
    // bottom cap
    unsigned bc = (unsigned)V.size();
    V.push_back({{0, 0, 0}, {0, -1, 0}, {0.5f, 0.5f}});
    for (int i = 0; i <= sl; i++)
    {
        float th = 2 * PI * i / sl;
        V.push_back({{0.5f * cosf(th), 0, 0.5f * sinf(th)}, {0, -1, 0}, {0.5f + 0.5f * cosf(th), 0.5f + 0.5f * sinf(th)}});
    }
    for (int i = 0; i < sl; i++)
        I.insert(I.end(), {bc, bc + 2 + i, bc + 1 + i});
    return m.upload(V, I);
}

// Torus (cup rim)
Mesh MeshFactory::makeTorus(float R, float r, int sl, int st) {
    vector<Vertex> V; vector<unsigned> I;
    for(int i=0;i<=sl;i++){
        float th=2*PI*i/sl;
        for(int j=0;j<=st;j++){
            float ph=2*PI*j/st;
            vec3 n={cosf(th)*cosf(ph),sinf(ph),sinf(th)*cosf(ph)};
            vec3 p={(R+r*cosf(ph))*cosf(th),r*sinf(ph),(R+r*cosf(ph))*sinf(th)};
            V.push_back({p,n,{(float)i/sl,(float)j/st}});
        }
    }
    for(int i=0;i<sl;i++)
        for(int j=0;j<st;j++){
            unsigned a=i*(st+1)+j,b=a+1,c=(i+1)*(st+1)+j,d=c+1;
            I.insert(I.end(),{a,c,b,b,c,d});
        }
    return m.upload(V,I);
}

// Trapezoidal property grass (wider at south)
Mesh MeshFactory::makeTrapezoid() {
     // North edge: z=-55, half-width=40 | South edge: z=+55, half-width=50
    vector<Vertex> V={
        {{-40,0,-55},{0,1,0},{0.03f,0}},
        {{ 40,0,-55},{0,1,0},{0.97f,0}},
        {{ 50,0, 55},{0,1,0},{1.00f,1}},
        {{-50,0, 55},{0,1,0},{0.00f,1}},
    };
    return m.upload(V,{0,1,2,0,2,3});
}

Mesh MeshFactory::makeSkyboxMesh(){
     float v[]={
       -1, 1,-1,-1,-1,-1, 1,-1,-1, 1,-1,-1, 1, 1,-1,-1, 1,-1,
       -1,-1, 1,-1,-1,-1,-1, 1,-1,-1, 1,-1,-1, 1, 1,-1,-1, 1,
        1,-1,-1, 1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1,-1,-1,
       -1,-1, 1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1,-1,-1, 1,
       -1, 1,-1, 1, 1,-1, 1, 1, 1, 1, 1, 1,-1, 1, 1,-1, 1,-1,
       -1,-1,-1,-1,-1, 1, 1,-1,-1, 1,-1,-1,-1,-1, 1, 1,-1, 1,
    };
    GLuint vao,vbo;
    glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    Mesh m; 
    m.setVAO(vao);
    m.setVBO(vbo);
    m.setEBO(0);
    m.setCount(36);
    
    return m;
}