/* ============================================================
   COS344 HA — Mini Golf  (clean base: 1 hole, 1 restaurant)
   Student: u14439141  Team: Ray Tracers

   Controls:
     WASD       fly drone (drone-view) / move observer (external-view)
     Q / E      drone/camera down / up
     Scroll     zoom in / out
     Right-drag smooth mouse look (steers drone or observer)
     F          toggle drone-cam / external-cam
     Space      spawn ball at tee
     G          toggle aim mode (ball must be still)
     LMB-drag   aim + power (in aim mode)
     Release    fire
     O          ortho / perspective
     R          reset camera
     KP+ / KP-  step time of day
     T          toggle auto time
     Arrow keys tilt spotlight direction (night only)
     Escape     quit
   ============================================================ */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "Mesh.h"
#include "Hole1.h"
#include "Hole2.h"
#include "Hole3.h"
#include "Hole4.h"
#include "Hole5.h"
#include "Hole6.h"
#include "Hole7.h"
#include "Hole8.h"
#include "Hole9.h"
#include "Hole10.h"
#include "Hole11.h"
#include "Hole12.h"

using namespace std;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

static const float PI    = 3.14159265358979f;
static const int   WIN_W = 1280;
static const int   WIN_H = 800;

// ─── Mesh helpers ────────────────────────────────────────────────────────────
Mesh upload(const vector<Vertex>& V, const vector<unsigned>& I){
    Mesh m; m.count=(int)I.size();
    glGenVertexArrays(1,&m.vao); glGenBuffers(1,&m.vbo); glGenBuffers(1,&m.ebo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER,m.vbo);
    glBufferData(GL_ARRAY_BUFFER,V.size()*sizeof(Vertex),V.data(),GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,I.size()*sizeof(unsigned),I.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,pos));  glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,norm)); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,uv));   glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    return m;
}
static void freeMesh(Mesh& m){
    glDeleteBuffers(1,&m.vbo); glDeleteBuffers(1,&m.ebo);
    glDeleteVertexArrays(1,&m.vao);
}

// Flat pentagon in XZ plane (triangle fan, circumradius 1, south vertex at +z)
Mesh makePentagonFloor(){
    vector<Vertex> V; vector<unsigned> I;
    V.push_back({{0,0,0},{0,1,0},{0.5f,0.5f}});
    for(int k=0;k<=5;k++){
        float t=PI/2.0f+k*(2.0f*PI/5.0f);
        float c=cosf(t), s=sinf(t);
        V.push_back({{c,0,s},{0,1,0},{c*0.5f+0.5f,s*0.5f+0.5f}});
    }
    for(int k=0;k<5;k++) I.insert(I.end(),{0,(unsigned)(k+1),(unsigned)(k+2)});
    return upload(V,I);
}

// Flat circle in XZ plane (triangle fan, radius 1)
Mesh makeCircle(int N){
    vector<Vertex> V; vector<unsigned> I;
    V.push_back({{0,0,0},{0,1,0},{0.5f,0.5f}});
    for(int i=0;i<=N;i++){
        float t=2*PI*i/N;
        float c=cosf(t), s=sinf(t);
        V.push_back({{c,0,s},{0,1,0},{c*0.5f+0.5f,s*0.5f+0.5f}});
    }
    for(int i=0;i<N;i++) I.insert(I.end(),{0,(unsigned)(i+1),(unsigned)(i+2)});
    return upload(V,I);
}

// Curved arc wall
Mesh makeArcWall(float R, float WTH, float WH, float tStart, float tEnd, int N){
    vector<Vertex> V; vector<unsigned> I;
    float Ro = R + WTH*0.5f, Ri = R - WTH*0.5f;
    float dt = (tEnd - tStart) / N;
    for(int i=0; i<=N; i++){
        float t=tStart+i*dt, ct=cosf(t), st=sinf(t);
        vec3 no={ ct,0, st}, ni={-ct,0,-st};
        V.push_back({{Ro*ct,  0, Ro*st}, no, {(float)i/N, 0}});
        V.push_back({{Ro*ct, WH, Ro*st}, no, {(float)i/N, 1}});
        V.push_back({{Ri*ct,  0, Ri*st}, ni, {(float)i/N, 0}});
        V.push_back({{Ri*ct, WH, Ri*st}, ni, {(float)i/N, 1}});
    }
    for(int i=0; i<N; i++){
        unsigned b=4*i;
        I.insert(I.end(),{b,   b+1, b+4, b+1, b+5, b+4});
        I.insert(I.end(),{b+2, b+6, b+3, b+3, b+6, b+7});
        I.insert(I.end(),{b+1, b+3, b+5, b+3, b+7, b+5});
    }
    return upload(V,I);
}

// Filled arc strip in XZ plane
Mesh makeArcFloor(float Ri, float Ro, float tStart, float tEnd, int N){
    vector<Vertex> V; vector<unsigned> I;
    float dt=(tEnd-tStart)/N;
    for(int i=0;i<=N;i++){
        float t=tStart+i*dt, ct=cosf(t), st=sinf(t);
        float u=(float)i/N;
        V.push_back({{Ri*ct,0,Ri*st},{0,1,0},{u,0}});
        V.push_back({{Ro*ct,0,Ro*st},{0,1,0},{u,1}});
    }
    for(int i=0;i<N;i++){
        unsigned b=2*i;
        I.insert(I.end(),{b,b+2,b+1, b+1,b+2,b+3});
    }
    return upload(V,I);
}

// Flat triangle in XZ plane
Mesh makeTriFloor(vec3 a, vec3 b, vec3 c){
    vector<Vertex> V={
        {a,{0,1,0},{0.0f,0.5f}},
        {b,{0,1,0},{0.0f,1.0f}},
        {c,{0,1,0},{1.0f,0.5f}},
    };
    return upload(V,{0,1,2});
}

// Horizontal quad in XZ plane
Mesh makeQuad(){
    vector<Vertex> V={
        {{-0.5f,0,-0.5f},{0,1,0},{0,0}},
        {{ 0.5f,0,-0.5f},{0,1,0},{1,0}},
        {{ 0.5f,0, 0.5f},{0,1,0},{1,1}},
        {{-0.5f,0, 0.5f},{0,1,0},{0,1}},
    };
    return upload(V,{0,1,2,0,2,3});
}

// Vertical quad in XY plane, centred, 1×1
// UVs: v=0 at top, v=1 at bottom (OpenGL origin = bottom-left, stb_image origin = top-left)
static Mesh makeVQuad(){
    vector<Vertex> V={
        {{-0.5f,-0.5f,0},{0,0,1},{0,1}},
        {{ 0.5f,-0.5f,0},{0,0,1},{1,1}},
        {{ 0.5f, 0.5f,0},{0,0,1},{1,0}},
        {{-0.5f, 0.5f,0},{0,0,1},{0,0}},
    };
    return upload(V,{0,1,2,0,2,3});
}

// Vertical quad that samples only the right 60% of a texture atlas (frond portion of lpalm1.png)
static Mesh makeLeafVQuad(){
    const float u0 = 0.40f, u1 = 1.0f;
    vector<Vertex> V={
        {{-0.5f,-0.5f,0},{0,0,1},{u0,1}},
        {{ 0.5f,-0.5f,0},{0,0,1},{u1,1}},
        {{ 0.5f, 0.5f,0},{0,0,1},{u1,0}},
        {{-0.5f, 0.5f,0},{0,0,1},{u0,0}},
    };
    return upload(V,{0,1,2,0,2,3});
}

// Unit box [-0.5,0.5]^3
Mesh makeBox(){
    vector<Vertex> V; vector<unsigned> I;
    struct F{ vec3 n,u,v,o; };
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
    return upload(V,I);
}

// Sphere
Mesh makeSphere(int st, int sl){
    vector<Vertex> V; vector<unsigned> I;
    for(int i=0;i<=st;i++){
        float phi=PI*i/st;
        for(int j=0;j<=sl;j++){
            float th=2*PI*j/sl;
            vec3 n={sinf(phi)*cosf(th),cosf(phi),sinf(phi)*sinf(th)};
            V.push_back({n,n,{(float)j/sl,(float)i/st}});
        }
    }
    for(int i=0;i<st;i++)
        for(int j=0;j<sl;j++){
            unsigned a=i*(sl+1)+j,b=a+1,c=(i+1)*(sl+1)+j,d=c+1;
            I.insert(I.end(),{a,c,b,b,c,d});
        }
    return upload(V,I);
}

// Cylinder
Mesh makeCylinder(int sl){
    vector<Vertex> V; vector<unsigned> I;
    for(int i=0;i<=sl;i++){
        float th=2*PI*i/sl;
        vec3 n={cosf(th),0,sinf(th)};
        V.push_back({{0.5f*cosf(th),0,0.5f*sinf(th)},n,{(float)i/sl,0}});
        V.push_back({{0.5f*cosf(th),1,0.5f*sinf(th)},n,{(float)i/sl,1}});
    }
    for(int i=0;i<sl;i++){
        unsigned a=2*i,b=2*i+1,c=2*(i+1),d=2*(i+1)+1;
        I.insert(I.end(),{a,b,c,b,d,c});
    }
    unsigned tc=(unsigned)V.size();
    V.push_back({{0,1,0},{0,1,0},{0.5f,0.5f}});
    for(int i=0;i<=sl;i++){
        float th=2*PI*i/sl;
        V.push_back({{0.5f*cosf(th),1,0.5f*sinf(th)},{0,1,0},
                     {0.5f+0.5f*cosf(th),0.5f+0.5f*sinf(th)}});
    }
    for(int i=0;i<sl;i++) I.insert(I.end(),{tc,tc+1+i,tc+2+i});
    unsigned bc=(unsigned)V.size();
    V.push_back({{0,0,0},{0,-1,0},{0.5f,0.5f}});
    for(int i=0;i<=sl;i++){
        float th=2*PI*i/sl;
        V.push_back({{0.5f*cosf(th),0,0.5f*sinf(th)},{0,-1,0},
                     {0.5f+0.5f*cosf(th),0.5f+0.5f*sinf(th)}});
    }
    for(int i=0;i<sl;i++) I.insert(I.end(),{bc,bc+2+i,bc+1+i});
    return upload(V,I);
}

// Torus
Mesh makeTorus(float R, float r, int sl, int st){
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
    return upload(V,I);
}

// Trapezoidal property grass
static Mesh makeTrapezoid(){
    vector<Vertex> V={
        {{-40,0,-55},{0,1,0},{0.03f,0}},
        {{ 40,0,-55},{0,1,0},{0.97f,0}},
        {{ 50,0, 55},{0,1,0},{1.00f,1}},
        {{-50,0, 55},{0,1,0},{0.00f,1}},
    };
    return upload(V,{0,1,2,0,2,3});
}

// Tapered cylinder: radius varies linearly from rBot (y=0) to rTop (y=1).
// Normals are tilted to match the cone-frustum surface, giving correct shading.
static Mesh makeTaperedCylinder(float rBot, float rTop, int sl = 18){
    vector<Vertex> V; vector<unsigned> I;
    float slope = rBot - rTop;  // radius decrease per unit height
    for(int i = 0; i <= sl; i++){
        float th = 2*PI*i/sl, ct = cosf(th), st = sinf(th);
        vec3 n = glm::normalize(vec3(ct, slope, st));
        V.push_back({{rBot*ct,   0, rBot*st}, n, {(float)i/sl, 0}});
        V.push_back({{rTop*ct, 1.f, rTop*st}, n, {(float)i/sl, 1}});
    }
    for(int i = 0; i < sl; i++){
        unsigned a=2*i, b=2*i+1, c=2*(i+1), d=2*(i+1)+1;
        I.insert(I.end(), {a, b, c, b, d, c});
    }
    return upload(V, I);
}

// River / pond strip mesh — flat XZ strip following a centerline with per-point width.
// yOff: world-space Y at which all vertices are placed.
static Mesh makeRiverStrip(const vector<vec3>& pts, const vector<float>& widths, float yOff){
    vector<Vertex> V; vector<unsigned> I;
    int n = (int)pts.size();
    for(int i = 0; i < n; i++){
        vec3 dir;
        if(i == 0)        dir = glm::normalize(pts[1]   - pts[0]);
        else if(i == n-1) dir = glm::normalize(pts[n-1] - pts[n-2]);
        else              dir = glm::normalize(pts[i+1] - pts[i-1]);
        vec3 perp = vec3(-dir.z, 0.f, dir.x);   // perpendicular in XZ plane
        float hw = widths[i] * 0.5f;
        float u  = (float)i / (n - 1);
        vec3 L = {pts[i].x - perp.x*hw, yOff, pts[i].z - perp.z*hw};
        vec3 R = {pts[i].x + perp.x*hw, yOff, pts[i].z + perp.z*hw};
        V.push_back({L, {0,1,0}, {u, 0.f}});
        V.push_back({R, {0,1,0}, {u, 1.f}});
    }
    for(int i = 0; i < n-1; i++){
        unsigned b = 2*i;
        I.insert(I.end(), {b, b+2, b+1, b+1, b+2, b+3});
    }
    return upload(V, I);
}

static GLuint loadTexture(const char* path){
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int w,h,n;
    unsigned char* data = stbi_load(path, &w, &h, &n, 0);
    if(data){
        GLenum fmt = (n==4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D,0,fmt,w,h,0,fmt,GL_UNSIGNED_BYTE,data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        printf("Failed to load texture: %s\n", path);
    }
    stbi_image_free(data);
    return tex;
}

// ─── Globals ──────────────────────────────────────────────────────────────────
static constexpr float PLAT_H = 0.85f;   // restaurant plateau height

static GLuint    gProg, skyProg, skyTex;
static Mesh      mQuad, mBox, mSphere, mCylinder, mTorus, mTrap, mSkybox, mCircle;
static Mesh      mTerrainHills;
static Mesh      mVQuad;              // vertical quad for billboards
static Mesh      mLeafVQuad;          // vertical quad sampling frond half of lpalm1 atlas
static Mesh      mRockyBoulder;       // low-poly displaced sphere
static Mesh      mRiver, mRiverBank;
static Mesh      mTaperedCyl;
static Mesh      mThickCyl;           // short, thick palm trunk (low palm)
static Mesh      mRestaurantPlateau;  // raised terrain around restaurant
static Mesh      mTerrainWest;        // elongated hill bank along west edge

// ─── Texture handles ──────────────────────────────────────────────────────────
static GLuint texRock[8];       // rocks/rock1-8.bmp
static GLuint texBark;          // rocks/wood4.bmp  (palm trunk bark)
static GLuint texPalmCrown[3];  // tropical/t_palm_shrubN/palmsN.png  (leaf crown)
static GLuint texWeed[3];       // tropical/t_trop_weedN/tropwN.png
static GLuint texShrub[2];      // tropical/t_trop_shrubN/tropsN.png
static GLuint texConcrete;      // rocks/floor1.bmp (concrete slabs)
static GLuint texAgave;         // tropical/t_agave1/agava1.png
static GLuint texFlax;          // tropical/t_flax1_green/flax1g.png
static GLuint texLowPalmCrown;  // tropical/t_lowpalm1/lpalm1.png
static GLFWwindow* gWin = nullptr;
static float     timeOfDay = 0.45f;
static float     todSpeed  = 0.005f;

// ─── Lamp post positions (world XZ, y=0 base) ────────────────────────────────
static vec3 LAMP_POSITIONS[] = {
    // Holes 1-2 (near cups and connecting path)
    {-30.5f, 0.f,  31.f},   // H1 cup, east side
    {-35.5f, 0.f,  31.f},   // H1 cup, west side
    {-30.5f, 0.f,  42.f},   // H1 tee area
    {-30.5f, 0.f,  24.f},   // between H1 and H2
    {-30.5f, 0.f,  13.f},   // H2 cup, east side
    {-35.5f, 0.f,  13.f},   // H2 cup, west side
    // Holes 3-5
    {-30.5f, 0.f,   4.f},   // between H2 and H3
    {-30.5f, 0.f,  -7.f},   // H3 cup area
    {-35.5f, 0.f, -20.f},   // H4 cup area
    {-30.5f, 0.f, -35.f},   // H5 cup area
    // Holes 6-8 (bottom section)
    {-35.5f, 0.f, -44.f},   // H6 tee/fairway
    {-21.f,  0.f, -49.f},   // H6 cup area
    {  4.f,  0.f, -50.f},   // H7 cup area
    { 10.f,  0.f, -48.f},   // between H7 and H8
    { 22.f,  0.f, -49.f},   // H8 cup area
    // Restaurant area (corner lamps — on the plateau top)
    { -6.f,  PLAT_H,   5.f},
    {  6.f,  PLAT_H,   5.f},
    { -6.f,  PLAT_H,  -5.f},
    {  6.f,  PLAT_H,  -5.f},
    // Pond lamp (right bank)
    { 24.f,  0.f,  15.f},
};
static const int LAMP_COUNT = (int)(sizeof(LAMP_POSITIONS)/sizeof(LAMP_POSITIONS[0]));

// ─── Hill terrain (upper-right corner, x=33..49, z=38..52) ──────────────────
struct TerHill { float x, z, h, sig; };
static const TerHill TERHILLS[] = {
    {38.f, 43.f, 2.5f, 4.0f},
    {45.f, 41.f, 2.0f, 3.5f},
    {36.f, 48.f, 2.3f, 3.8f},
    {47.f, 48.f, 1.7f, 3.0f},
    {42.f, 50.f, 1.9f, 3.5f},
};
static const int TERHILL_N = (int)(sizeof(TERHILLS)/sizeof(TERHILLS[0]));
static float terrainH(float x, float z){
    float h = 0;
    for(int i = 0; i < TERHILL_N; i++){
        float dx = x - TERHILLS[i].x, dz = z - TERHILLS[i].z;
        h += TERHILLS[i].h * expf(-(dx*dx + dz*dz)/(2.f*TERHILLS[i].sig*TERHILLS[i].sig));
    }
    return h;
}
// Returns the actual rendered (edge-faded) terrain height — use this for boulders near the mesh boundary
static float hillH(float x, float z){
    const float x0=33.f, z0=38.f, w=16.f, d=14.f;
    float r=0.14f;
    auto fade1 = [&](float t) -> float {
        float f = (t < r) ? t/r : (t > 1.f-r) ? (1.f-t)/r : 1.f;
        return f*f*(3.f-2.f*f);
    };
    float px = glm::clamp((x-x0)/w, 0.f, 1.f);
    float pz = glm::clamp((z-z0)/d, 0.f, 1.f);
    return terrainH(x,z) * fade1(px) * fade1(pz);
}

// ─── West hill banks (x=-49..-35, z=-46..44) ─────────────────────────────────
static const TerHill TERHILLS_WEST[] = {
    {-44.f, -38.f, 1.8f, 3.5f},
    {-45.f, -24.f, 1.5f, 3.5f},
    {-44.f,  -9.f, 1.7f, 3.0f},
    {-45.f,   6.f, 1.4f, 3.5f},
    {-44.f,  19.f, 1.6f, 3.0f},
    {-45.f,  31.f, 1.5f, 3.5f},
    {-44.f,  40.f, 1.8f, 3.0f},
};
static const int TERHILL_WEST_N = (int)(sizeof(TERHILLS_WEST)/sizeof(TERHILLS_WEST[0]));
static float terrainHW(float x, float z){
    float h = 0;
    for(int i = 0; i < TERHILL_WEST_N; i++){
        float dx = x - TERHILLS_WEST[i].x, dz = z - TERHILLS_WEST[i].z;
        h += TERHILLS_WEST[i].h * expf(-(dx*dx+dz*dz)/(2.f*TERHILLS_WEST[i].sig*TERHILLS_WEST[i].sig));
    }
    return h;
}

// ─── Drone state ──────────────────────────────────────────────────────────────
struct Drone {
    vec3  pos   = {-33.f, 12.f, 20.f};
    float yaw   = 0.f;
    float pitch = -0.4f;
} drone;

static bool  droneView   = true;   // true = first-person drone-cam
static float spotYawOff  = 0.f;    // arrow key offsets from drone facing
static float spotPitOff  = 0.f;
static float propAngle   = 0.f;    // spinning propellers

// ─── Camera ──────────────────────────────────────────────────────────────────
struct Camera {
    vec3  pos   = {-33, 25, 55};
    float yaw   = 0;
    float pitch = -0.50f;
    float fov   = 60.f;
    float spd   = 18.f;
    bool  ortho = false;

    vec3 fwd() const {
        return {sinf(yaw)*cosf(pitch), sinf(pitch), -cosf(yaw)*cosf(pitch)};
    }
    mat4 view() const { return glm::lookAt(pos, pos+fwd(), {0,1,0}); }
    mat4 proj(float a) const {
        if(ortho){ float h=18,w=h*a; return glm::ortho(-w,w,-h,h,.1f,400.f); }
        return glm::perspective(glm::radians(fov),a,.1f,400.f);
    }
} cam;

// ─── Ball ─────────────────────────────────────────────────────────────────────
static const float BALL_R  =  0.08f;
static const float MAX_AIM =  6.0f;

static int   gCurrentHole  = 1;
static Hole* gHoles[13]   = {};   // indices 1-12
static float windmillAngle = 0.f; // Hole 11 windmill rotor

struct Ball {
    vec3  pos    = {H1_CX, BALL_R, H1_CZ+5.5f};
    vec3  vel    = {0,0,0};
    bool  active = false;
    bool  moving = false;
    bool  inHole = false;
    int   strokes= 0;

    void update(float dt){
        if(!active||!moving) return;
        pos += vel*dt;
        vel *= powf(0.965f, dt*60.f);
        if(glm::length(vel)<0.02f){ vel={0,0,0}; moving=false; }
        wallCollide();
    }

    void wallCollide(){
        if(gHoles[gCurrentHole]) gHoles[gCurrentHole]->wallCollide(pos, vel);
    }

    bool nearCup() const {
        return gHoles[gCurrentHole] && gHoles[gCurrentHole]->nearCup(pos);
    }
} ball;

// ─── Aim state ────────────────────────────────────────────────────────────────
static bool  aimMode   = false;
static vec3  aimTarget = {0,0,0};

// ─── Mouse state ─────────────────────────────────────────────────────────────
static bool   rmbDown   = false;
static bool   skipFirst = true;
static double lastMX    = 0, lastMY = 0;

// ─── draw() shorthand ────────────────────────────────────────────────────────
void draw(const Mesh& m, const mat4& model, const mat4& vp, int surf){
    mat4 mvp = vp*model;
    glUniformMatrix4fv(glGetUniformLocation(gProg,"uMVP"),  1,GL_FALSE,glm::value_ptr(mvp));
    glUniformMatrix4fv(glGetUniformLocation(gProg,"uModel"),1,GL_FALSE,glm::value_ptr(model));
    glUniform1i(glGetUniformLocation(gProg,"uSurface"),surf);
    glBindVertexArray(m.vao);
    glDrawElements(GL_TRIANGLES,m.count,GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
}

// Unproject mouse screen coords to the y=0 ground plane
static vec3 mouseToGround(double mx, double my){
    float asp = (float)WIN_W / WIN_H;
    float nx  = (float)(2.0*mx/WIN_W - 1.0);
    float ny  = (float)(1.0 - 2.0*my/WIN_H);
    vec4 rayEye = glm::inverse(cam.proj(asp)) * vec4(nx, ny, -1.f, 1.f);
    rayEye = {rayEye.x, rayEye.y, -1.f, 0.f};
    vec3 rayWorld = glm::normalize(vec3(glm::inverse(cam.view()) * rayEye));
    if(fabsf(rayWorld.y) < 0.001f) return ball.pos;
    float t = -cam.pos.y / rayWorld.y;
    if(t < 0.f) return ball.pos;
    return cam.pos + rayWorld * t;
}

static void updateAimTarget(double mx, double my){
    vec3 raw  = mouseToGround(mx, my);
    vec3 diff = {raw.x - ball.pos.x, 0.f, raw.z - ball.pos.z};
    float d   = glm::length(diff);
    if(d > 0.001f && d > MAX_AIM)
        aimTarget = {ball.pos.x + diff.x/d*MAX_AIM, 0.f, ball.pos.z + diff.z/d*MAX_AIM};
    else
        aimTarget = {raw.x, 0.f, raw.z};
}

// ─── Drone draw ───────────────────────────────────────────────────────────────
static void drawDrone(const mat4& vp){
    vec3 p = drone.pos;
    mat4 baseRot = glm::rotate(mat4(1), drone.yaw, {0,1,0});

    // Fuselage — flattened box
    {
        mat4 m = glm::translate(mat4(1), p) * baseRot;
        m = glm::scale(m, {1.4f, 0.28f, 0.9f});
        draw(mBox, m, vp, 24);
    }
    // Central dome (sphere on top)
    {
        mat4 m = glm::translate(mat4(1), p + vec3(0, 0.22f, 0));
        m = glm::scale(m, {0.38f, 0.22f, 0.38f});
        draw(mSphere, m, vp, 24);
    }

    // 4 arms + motor + propellers
    float armAngles[4] = {PI*0.25f, PI*0.75f, PI*1.25f, PI*1.75f};
    for(int i=0; i<4; i++){
        float aa  = armAngles[i] + drone.yaw;
        float ax  = cosf(aa) * 0.9f;
        float az  = sinf(aa) * 0.9f;
        vec3  tip = p + vec3(ax, 0, az);

        // Arm
        {
            vec3 mid = p + vec3(ax*0.5f, 0, az*0.5f);
            mat4 m = glm::translate(mat4(1), mid);
            m = glm::rotate(m, aa, {0,1,0});
            m = glm::scale(m, {0.12f, 0.08f, 0.9f});
            draw(mBox, m, vp, 24);
        }
        // Motor mount (small sphere at tip)
        {
            mat4 m = glm::translate(mat4(1), tip);
            m = glm::scale(m, {0.12f, 0.12f, 0.12f});
            draw(mSphere, m, vp, 24);
        }
        // 2 propeller blades per motor
        for(int b=0; b<2; b++){
            float ba  = propAngle + b*PI + aa;
            float bx  = cosf(ba) * 0.32f;
            float bz  = sinf(ba) * 0.32f;
            vec3  bpos = tip + vec3(bx, 0.10f, bz);
            mat4  m   = glm::translate(mat4(1), bpos);
            m = glm::rotate(m, ba, {0,1,0});
            m = glm::scale(m, {0.60f, 0.04f, 0.14f});
            draw(mBox, m, vp, 25);
        }
    }
}

// ─── Lamp post ───────────────────────────────────────────────────────────────
static void drawLampPost(vec3 pos, const mat4& vp){
    // Pole — dark metal cylinder
    {
        mat4 m = glm::translate(mat4(1), pos);
        m = glm::scale(m, {0.10f, 4.0f, 0.10f});
        draw(mCylinder, m, vp, 13);
    }
    // Horizontal arm
    {
        mat4 m = glm::translate(mat4(1), pos + vec3(0, 4.0f, 0));
        m = glm::scale(m, {0.8f, 0.08f, 0.08f});
        draw(mBox, m, vp, 13);
    }
    // Lamp globe (emissive)
    {
        mat4 m = glm::translate(mat4(1), pos + vec3(0.4f, 4.0f, 0));
        m = glm::scale(m, {0.25f, 0.25f, 0.25f});
        draw(mSphere, m, vp, 14);
    }
}

// ─── Rocky boulder mesh (low-poly sphere with hash displacement) ──────────────
static float boulderHash(float x, float y, float z){
    float v = sinf(x*127.1f + y*311.7f + z*74.3f) * 43758.5453f;
    return v - floorf(v);
}
static Mesh makeRockyBoulder(){
    const int st=9, sl=14;
    const float rough=0.20f;
    vector<Vertex> V; vector<unsigned> I;
    for(int i=0; i<=st; i++){
        float phi=PI*i/st;
        for(int j=0; j<=sl; j++){
            float th=2*PI*j/sl;
            vec3 base={sinf(phi)*cosf(th), cosf(phi), sinf(phi)*sinf(th)};
            float n=boulderHash(base.x, base.y, base.z);
            float disp=1.0f + rough*(n*2.0f-1.0f);
            vec3 p=base*disp;
            V.push_back({p, base, {(float)j/sl, (float)i/st}});
        }
    }
    for(int i=0; i<st; i++)
        for(int j=0; j<sl; j++){
            unsigned a=i*(sl+1)+j, b=a+1, c=(i+1)*(sl+1)+j, d=c+1;
            I.insert(I.end(),{a,c,b,b,c,d});
        }
    return upload(V, I);
}

// ─── Hill terrain mesh builder ───────────────────────────────────────────────
// Heights fade smoothly to 0 at the mesh boundary so it blends with flat ground
static float edgeFade(float t){
    // t = 0..1 normalised; returns 0 at edges, 1 in the interior
    float r = 0.14f;  // fade zone: outer 14% per edge (≈2.2 units on a 16-unit mesh)
    float f = (t < r) ? t/r : (t > 1.f-r) ? (1.f-t)/r : 1.f;
    return f*f*(3.f - 2.f*f);  // smoothstep
}
static Mesh makeHillTerrain(float x0, float z0, float w, float d, int nx, int nz){
    vector<Vertex> V; vector<unsigned> I;
    float ddx = w/nx, ddz = d/nz, eps = 0.4f;
    for(int iz = 0; iz <= nz; iz++){
        for(int ix = 0; ix <= nx; ix++){
            float x = x0 + ix*ddx, z = z0 + iz*ddz;
            float fade = edgeFade((float)ix/nx) * edgeFade((float)iz/nz);
            float raw  = terrainH(x, z);
            float y    = raw * fade;          // exactly 0 at borders → no seam

            // Finite-difference normal from the faded height field
            auto fh = [&](float fx, float fz) -> float {
                float px = (fx-x0)/w, pz = (fz-z0)/d;
                return terrainH(fx,fz) * edgeFade(glm::clamp(px,0.f,1.f))
                                       * edgeFade(glm::clamp(pz,0.f,1.f));
            };
            float hxp = fh(x+eps,z), hxn = fh(x-eps,z);
            float hzp = fh(x,z+eps), hzn = fh(x,z-eps);
            vec3 norm = glm::normalize(vec3(-(hxp-hxn)/(2*eps), 1.f, -(hzp-hzn)/(2*eps)));
            V.push_back({{x,y,z}, norm, {(float)ix/nx, (float)iz/nz}});
        }
    }
    for(int iz = 0; iz < nz; iz++)
        for(int ix = 0; ix < nx; ix++){
            unsigned a = iz*(nx+1)+ix;
            I.insert(I.end(),{a, a+(unsigned)(nx+1), a+1,
                               a+1, a+(unsigned)(nx+1), a+(unsigned)(nx+1)+1});
        }
    return upload(V, I);
}

// West-edge hill bank: elongated strip of rolling mounds along the west boundary.
// Edge fades applied on east side (stops at holes), south end (meets river bank),
// and north end so every edge blends smoothly to flat ground.
static Mesh makeWestHillTerrain(){
    const float x0=-49.f, z0=-55.f, w=12.f, d=99.f;  // east limit x=-37, north z=-55
    const int   nx=24,    nz=99;
    auto ss = [](float t) -> float {          // smoothstep 0→1
        float f = glm::clamp(t, 0.f, 1.f);
        return f * f * (3.f - 2.f*f);
    };
    // fullH applies all three edge fades to the raw Gaussian terrain
    auto fullH = [&](float x, float z) -> float {
        float fe = ss((-37.f - x)   / 5.f);   // drops to 0 at x=-37 (hole boundary)
        float fs = ss(( 44.f - z)   / 9.f);   // drops to 0 at z=44  (south edge / river)
        float fn = ss((z - (-55.f)) / 5.f);   // drops to 0 at z=-55 (north course edge)
        return terrainHW(x, z) * fe * fs * fn;
    };
    vector<Vertex> V; vector<unsigned> I;
    float ddx=w/nx, ddz=d/nz, eps=0.4f;
    for(int iz=0; iz<=nz; iz++){
        for(int ix=0; ix<=nx; ix++){
            float x=x0+ix*ddx, z=z0+iz*ddz;
            float y    = fullH(x, z);
            float hxp  = fullH(x+eps, z), hxn = fullH(x-eps, z);
            float hzp  = fullH(x, z+eps), hzn = fullH(x, z-eps);
            vec3 norm  = glm::normalize(vec3(-(hxp-hxn)/(2*eps), 1.f, -(hzp-hzn)/(2*eps)));
            V.push_back({{x, y, z}, norm, {(float)ix/nx, (float)iz/nz}});
        }
    }
    for(int iz=0; iz<nz; iz++)
        for(int ix=0; ix<nx; ix++){
            unsigned a=iz*(nx+1)+ix;
            I.insert(I.end(),{a, a+(unsigned)(nx+1), a+1,
                              a+1, a+(unsigned)(nx+1), a+(unsigned)(nx+1)+1});
        }
    return upload(V,I);
}

// Restaurant plateau: smooth smoothstep rise from ground level up to PLAT_H.
// Flat top covers the building + all concrete slabs (x∈[-8,9], z∈[-7,8]).
// Slope fades to zero over 6 units on every side.
static Mesh makeRestaurantPlateau(){
    const float cx = 0.5f, cz = 0.5f;   // centre of the flat region
    const float px = 8.5f, pz = 7.5f;   // half-extents of flat top
    const float sw = 6.0f;              // slope width (units)

    const float x0 = cx - px - sw;      // = -14
    const float z0 = cz - pz - sw;      // = -13
    const float w  = 2.f*(px + sw);     // = 29
    const float d  = 2.f*(pz + sw);     // = 27
    const int   nx = 58, nz = 54;

    auto platH = [&](float x, float z) -> float {
        float ax = fabsf(x - cx), az = fabsf(z - cz);
        float fx = glm::clamp((px + sw - ax) / sw, 0.f, 1.f);
        float fz = glm::clamp((pz + sw - az) / sw, 0.f, 1.f);
        float f  = glm::min(fx, fz);
        // East-side crop: stop at the river's west bank so the plateau
        // doesn't rise into the pond.  Bank runs from x≈14.5 at z=0 down
        // to x≈7.9 at z=12, matching the computed left-bank positions.
        if(x > cx){
            float x_bank = 14.5f - 0.55f * glm::clamp(z, 0.f, 12.f);
            float bank_fx = glm::clamp((x_bank - x) / 2.f, 0.f, 1.f);
            f = glm::min(f, bank_fx);
        }
        return PLAT_H * f * f * (3.f - 2.f*f);
    };

    vector<Vertex> V; vector<unsigned> I;
    float ddx = w/nx, ddz = d/nz, eps = 0.25f;
    for(int iz = 0; iz <= nz; iz++){
        for(int ix = 0; ix <= nx; ix++){
            float x = x0 + ix*ddx, z = z0 + iz*ddz;
            float y = platH(x, z) + 0.001f;   // +0.001 to sit above flat trapezoid
            float hxp = platH(x+eps, z), hxn = platH(x-eps, z);
            float hzp = platH(x, z+eps), hzn = platH(x, z-eps);
            vec3 norm = glm::normalize(vec3(-(hxp-hxn)/(2*eps), 1.f, -(hzp-hzn)/(2*eps)));
            V.push_back({{x, y, z}, norm, {(float)ix/nx, (float)iz/nz}});
        }
    }
    for(int iz = 0; iz < nz; iz++)
        for(int ix = 0; ix < nx; ix++){
            unsigned a = iz*(nx+1)+ix;
            I.insert(I.end(), {a, a+(unsigned)(nx+1), a+1,
                               a+1, a+(unsigned)(nx+1), a+(unsigned)(nx+1)+1});
        }
    return upload(V, I);
}

// Draw with texture bound (resets uUseTex=0 after use)
static void drawWithTex(const Mesh& mesh, const mat4& model, const mat4& vp, int surf, GLuint tex){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(glGetUniformLocation(gProg,"uTex"),    0);
    glUniform1i(glGetUniformLocation(gProg,"uUseTex"), 1);
    draw(mesh, model, vp, surf);
    glUniform1i(glGetUniformLocation(gProg,"uUseTex"), 0);
}

// Textured rugged boulder
static void drawBoulder(vec3 pos, float r, GLuint tex, const mat4& vp){
    mat4 m = glm::translate(mat4(1), pos);
    m = glm::scale(m, {r, r*0.70f, r*0.85f});
    drawWithTex(mRockyBoulder, m, vp, 10, tex);
}

// Palm tree: single tapered trunk (no segment joints) + crossed-billboard leaf crown
static void drawPalmTree(vec3 base, float height, GLuint barkTex, GLuint crownTex, const mat4& vp){
    // leanRad reproduces the same +X lean as the old 4-segment code (0.018*4 = 0.072 rad)
    const float leanRad = 0.072f;
    {
        mat4 m = glm::translate(mat4(1), base);
        m = glm::rotate(m, -leanRad, {0.f, 0.f, 1.f});  // negative → top tilts toward +X
        m = glm::scale(m, {1.f, height, 1.f});
        drawWithTex(mTaperedCyl, m, vp, 11, barkTex);
    }
    // Crown sits at the true tip of the leaned trunk
    vec3 crown = base + vec3(sinf(leanRad)*height, cosf(leanRad)*height, 0.f);
    float cw = height * 0.60f, ch = height * 0.50f;
    for(int k = 0; k < 3; k++){
        float ang = (float)k * (PI / 3.0f);
        mat4 m = glm::translate(mat4(1), crown + vec3(0, ch*0.35f, 0));
        m = glm::rotate(m, ang, {0,1,0});
        m = glm::scale(m, {cw, ch, 1.f});
        drawWithTex(mVQuad, m, vp, 12, crownTex);
    }
}

// Short, thick palm tree (low palm) — procedural bark trunk + leafy flaring crown.
// Trunk uses procedural surface 11 (palm bark rings) — avoids texture-wrapping
// artefacts on the wider cylinder. Crown uses the standard palm-shrub billboard
// sprite (texPalmCrown passed in) with radial displacement so every frond keeps
// its normal pointing upward (stays properly lit by the sun).
// Two layers: 8 upright primary fronds + 6 drooping outer fronds (bent away).
static void drawLowPalmTree(vec3 base, float height, GLuint crownTex, const mat4& vp){
    const float leanRad = 0.05f;
    {
        mat4 m = glm::translate(mat4(1), base);
        m = glm::rotate(m, -leanRad, {0.f, 0.f, 1.f});
        m = glm::scale(m, {1.f, height, 1.f});
        draw(mThickCyl, m, vp, 11);   // procedural palm-bark shader — no UV artefacts
    }
    vec3 crown = base + vec3(sinf(leanRad)*height, cosf(leanRad)*height, 0.f);

    // ── 8 primary fronds — displaced radially so they flare without tilting ───
    // Radial displacement keeps normals vertical → correct sun lighting.
    const float cw1 = height * 0.80f, ch1 = height * 0.60f;
    const float rad1 = cw1 * 0.30f;   // outward displacement from crown tip
    for(int k = 0; k < 8; k++){
        float ang = (float)k * (PI / 4.0f);
        float ox = sinf(ang) * rad1, oz = cosf(ang) * rad1;
        mat4 m = glm::translate(mat4(1), crown + vec3(ox, ch1 * 0.35f, oz));
        m = glm::rotate(m, ang, {0.f, 1.f, 0.f});
        m = glm::scale(m, {cw1, ch1, 1.f});
        drawWithTex(mVQuad, m, vp, 12, crownTex);
    }

    // ── 6 drooping outer fronds — displaced further out + slight downward tilt ─
    // Small Z-tilt (20°) simulates bend; kept modest so lighting stays decent.
    const float cw2 = height * 0.75f, ch2 = height * 0.68f;
    const float rad2 = cw2 * 0.45f;
    const float droop = glm::radians(20.f);
    for(int k = 0; k < 6; k++){
        float ang = (float)k * (2.f * PI / 6.f) + glm::radians(22.5f);
        float ox = sinf(ang) * rad2, oz = cosf(ang) * rad2;
        mat4 m = glm::translate(mat4(1), crown + vec3(ox, ch2 * 0.10f, oz));
        m = glm::rotate(m, ang,   {0.f, 1.f, 0.f});
        m = glm::rotate(m, droop, {0.f, 0.f, 1.f}); // gentle outward droop
        m = glm::scale(m, {cw2, ch2, 1.f});
        drawWithTex(mVQuad, m, vp, 12, crownTex);
    }
}

// Camera-facing cylindrical billboard (plant/tree sprite)
static void drawBillboard(vec3 base, float w, float h, GLuint tex, const mat4& vp){
    vec3 toCam = cam.pos - base;
    toCam.y = 0.f;
    float ang = atan2f(toCam.x, toCam.z);
    mat4 m = glm::translate(mat4(1), base + vec3(0, h*0.5f, 0));
    m = glm::rotate(m, ang, {0,1,0});
    m = glm::scale(m, {w, h, 1.f});
    drawWithTex(mVQuad, m, vp, 12, tex);
}

static void drawWalkway(vec3 a, vec3 b, float w, const mat4& vp){
    vec3 diff = b - a;
    float len = glm::length(diff);
    if(len < 0.01f) return;
    vec3 dir = diff / len;
    float ang = atan2f(dir.x, dir.z);
    vec3 mid = (a + b) * 0.5f;
    mat4 m = glm::translate(mat4(1), mid + vec3(0, 0.005f, 0));
    m = glm::rotate(m, ang, {0, 1, 0});
    m = glm::scale(m, {w, 1.f, len});
    draw(mQuad, m, vp, 3);
}

// ─── Callbacks ───────────────────────────────────────────────────────────────
static void cbScroll(GLFWwindow*, double, double dy){
    cam.fov = glm::clamp(cam.fov - (float)dy*2.5f, 10.f, 95.f);
}

static void cbMouseBtn(GLFWwindow*, int btn, int act, int){
    if(btn == GLFW_MOUSE_BUTTON_RIGHT){
        rmbDown = (act == GLFW_PRESS);
        skipFirst = true;
    }
    if(btn == GLFW_MOUSE_BUTTON_LEFT && aimMode){
        if(act == GLFW_PRESS){
            double x,y; glfwGetCursorPos(gWin,&x,&y);
            updateAimTarget(x, y);
        }
        if(act == GLFW_RELEASE && ball.active && !ball.moving){
            vec3 diff = {aimTarget.x - ball.pos.x, 0, aimTarget.z - ball.pos.z};
            float dist = glm::length(diff);
            if(dist > 0.05f){
                vec3 dir = diff / dist;
                float power = (dist / MAX_AIM) * 28.f;
                power = glm::max(power, 0.5f);
                ball.vel    = dir * power;
                ball.moving = true;
                ball.strokes++;
                aimMode = false;
            }
        }
    }
}

static void cbMouseMove(GLFWwindow*, double x, double y){
    if(aimMode && glfwGetMouseButton(gWin,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS)
        updateAimTarget(x, y);
    if(!rmbDown) return;
    if(skipFirst){ lastMX=x; lastMY=y; skipFirst=false; return; }

    float dx = (float)(x - lastMX) * 0.003f;
    float dy = (float)(y - lastMY) * 0.003f;
    lastMX=x; lastMY=y;

    if(droneView){
        // Steer the drone
        drone.yaw   += dx;
        drone.pitch -= dy;
        drone.pitch  = glm::clamp(drone.pitch, -1.4f, 1.4f);
    } else {
        // Steer the external observer camera
        cam.yaw   += dx;
        cam.pitch -= dy;
        cam.pitch  = glm::clamp(cam.pitch, -1.4f, 1.4f);
    }
}

static void cbKey(GLFWwindow* w, int key, int, int act, int){
    if(act != GLFW_PRESS) return;
    switch(key){
    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(w,true); break;
    case GLFW_KEY_O:      cam.ortho = !cam.ortho; break;
    case GLFW_KEY_R:
        cam.pos={-33,25,55}; cam.yaw=0; cam.pitch=-0.50f; cam.fov=60.f; break;

    // ── Toggle drone-cam / external-cam ──
    case GLFW_KEY_F:
        droneView = !droneView;
        if(droneView){
            // Lock camera to drone
            cam.pos   = drone.pos;
            cam.yaw   = drone.yaw;
            cam.pitch = drone.pitch;
        } else {
            // Step external cam back so you can see the drone
            cam.pos   = drone.pos + vec3(
                -sinf(drone.yaw)*8.f, 3.f, cosf(drone.yaw)*8.f);
            cam.yaw   = drone.yaw;
            cam.pitch = -0.25f;
        }
        break;

    case GLFW_KEY_G:
        if(ball.active && !ball.moving) aimMode = !aimMode; break;

    case GLFW_KEY_SPACE: {
        if(gHoles[gCurrentHole]){
            vec3 tp = gHoles[gCurrentHole]->getTeePos();
            ball.pos = tp; ball.vel = {0,0,0};
            ball.active = true; ball.moving = false;
            ball.inHole = false; ball.strokes = 0;
            aimMode = false;
        }
        break;
    }
    case GLFW_KEY_KP_ADD:
        timeOfDay = fmodf(timeOfDay+0.05f, 1.f); break;
    case GLFW_KEY_KP_SUBTRACT:
        timeOfDay = fmodf(timeOfDay+0.95f, 1.f); break;
    case GLFW_KEY_T:
        todSpeed = (todSpeed > 0) ? 0 : 0.005f; break;
    case GLFW_KEY_EQUAL:  cam.fov = glm::max(cam.fov-3.f, 10.f); break;
    case GLFW_KEY_MINUS:  cam.fov = glm::min(cam.fov+3.f, 95.f); break;

    // ── Spotlight direction (arrow keys) ──
    case GLFW_KEY_UP:    spotPitOff = glm::clamp(spotPitOff-0.1f,-1.4f, 0.f); break;
    case GLFW_KEY_DOWN:  spotPitOff = glm::clamp(spotPitOff+0.1f,-1.4f, 0.f); break;
    case GLFW_KEY_LEFT:  spotYawOff -= 0.15f; break;
    case GLFW_KEY_RIGHT: spotYawOff += 0.15f; break;
    }
}

// ─── Set lighting uniforms ────────────────────────────────────────────────────
static void setUniforms(float t, const mat4& vp){
    (void)vp;
    glUniform1f(glGetUniformLocation(gProg,"uTime"),      t);
    glUniform1f(glGetUniformLocation(gProg,"uTimeOfDay"), timeOfDay);
    glUniform1f(glGetUniformLocation(gProg,"uAmbient"),   0.30f);
    glUniform1i(glGetUniformLocation(gProg,"uUseTex"),    0);
    glUniform1i(glGetUniformLocation(gProg,"uTex"),       0);

    float sun = timeOfDay*2*PI;
    vec3 sdir = glm::normalize(vec3(cosf(sun), sinf(sun)*0.9f+0.1f, -0.4f));
    glUniform3fv(glGetUniformLocation(gProg,"uLightDir"),1,glm::value_ptr(sdir));

    vec3 scol = {1.f,0.95f,0.85f};
    if(timeOfDay<0.27f||timeOfDay>0.76f)    scol={0.07f,0.05f,0.04f};
    else if(timeOfDay<0.37f) scol=glm::mix(vec3(1,.5f,.2f),vec3(1,.95f,.85f),(timeOfDay-.27f)/.1f);
    else if(timeOfDay>0.67f) scol=glm::mix(vec3(1,.95f,.85f),vec3(1,.5f,.2f),(timeOfDay-.67f)/.09f);
    glUniform3fv(glGetUniformLocation(gProg,"uLightColor"),1,glm::value_ptr(scol));
    glUniform3fv(glGetUniformLocation(gProg,"uCamPos"),    1,glm::value_ptr(cam.pos));

    bool nightNow = (timeOfDay < 0.28f || timeOfDay > 0.72f);
    glUniform1i(glGetUniformLocation(gProg,"uLampsOn"),   nightNow ? 1 : 0);
    glUniform1i(glGetUniformLocation(gProg,"uLampCount"), LAMP_COUNT);
    {
        static vec3 lampWorldPos[32];
        for(int i = 0; i < LAMP_COUNT; i++)
            lampWorldPos[i] = LAMP_POSITIONS[i] + vec3(0, 3.8f, 0);
        glUniform3fv(glGetUniformLocation(gProg,"uLampPos"),   LAMP_COUNT, glm::value_ptr(lampWorldPos[0]));
    }
    vec3 lampCol = {1.0f, 0.92f, 0.65f};
    glUniform3fv(glGetUniformLocation(gProg,"uLampColor"), 1, glm::value_ptr(lampCol));

    // ── Spotlight ──
    int  spotOn   = nightNow ? 1 : 0;
    // Direction = drone facing + arrow key offsets, biased downward
    float sy = drone.yaw   + spotYawOff;
    float sp = drone.pitch + spotPitOff - 0.3f;
    vec3 spotDir = glm::normalize(vec3(
        sinf(sy)*cosf(sp), sinf(sp), -cosf(sy)*cosf(sp)));
    glUniform1i(glGetUniformLocation(gProg,"uSpotOn"),     spotOn);
    glUniform3fv(glGetUniformLocation(gProg,"uSpotPos"), 1,glm::value_ptr(drone.pos));
    glUniform3fv(glGetUniformLocation(gProg,"uSpotDir"), 1,glm::value_ptr(spotDir));
    glUniform1f(glGetUniformLocation(gProg,"uSpotCutoff"), cosf(glm::radians(18.f)));
    glUniform1f(glGetUniformLocation(gProg,"uSpotOuter"),  cosf(glm::radians(26.f)));
}

// Rotation semantics: rot=0 → person faces +Z; rot=PI → faces -Z;
//   rot=+PI/2 → faces +X (use for chair left of table); rot=-PI/2 → faces -X (right of table)
static void drawChair(vec3 pos, float rot, const mat4& vp){
    mat4 base = glm::translate(mat4(1), pos);
    base = glm::rotate(base, rot, {0.f, 1.f, 0.f});
    // 4 legs: mCylinder y=0..1, scaled to 0.65 → ground to seat bottom
    const float lx[4]={-0.18f, 0.18f,-0.18f, 0.18f};
    const float lz[4]={-0.16f,-0.16f, 0.16f, 0.16f};
    for(int i=0;i<4;i++){
        mat4 m = glm::translate(base, {lx[i], 0.f, lz[i]});
        m = glm::scale(m, {0.05f, 0.65f, 0.05f});
        draw(mCylinder, m, vp, 13);
    }
    // Seat: box centred at y=0.68 (top of legs + half thickness)
    { mat4 m = glm::translate(base, {0, 0.68f, 0}); m = glm::scale(m, {0.48f, 0.06f, 0.44f}); draw(mBox, m, vp, 6); }
    // Backrest: centred at y=1.00, offset behind seat (local -z)
    { mat4 m = glm::translate(base, {0, 1.00f, -0.19f}); m = glm::scale(m, {0.48f, 0.60f, 0.06f}); draw(mBox, m, vp, 6); }
}

static void drawTable(vec3 pos, const mat4& vp){
    // 4 legs: mCylinder y=0..1, scaled to 1.15
    const float lx[4]={-0.50f, 0.50f,-0.50f, 0.50f};
    const float lz[4]={-0.38f,-0.38f, 0.38f, 0.38f};
    for(int i=0;i<4;i++){
        mat4 m = glm::translate(mat4(1), pos + vec3(lx[i], 0.f, lz[i]));
        m = glm::scale(m, {0.06f, 1.15f, 0.06f});
        draw(mCylinder, m, vp, 13);
    }
    // Tabletop: box centred at y=1.18 (top of legs + half thickness)
    { mat4 m = glm::translate(mat4(1), pos + vec3(0, 1.18f, 0)); m = glm::scale(m, {1.2f, 0.06f, 0.90f}); draw(mBox, m, vp, 6); }
}

// ── Hole 11 windmill obstacle ─────────────────────────────────────────────────
// Faces the tee (west). Ball must time entry through the gap (z±0.60 of centre).
// Blades spin in the ZY-plane around the X axis. Brown shades bottom→top.
static void drawWindmill(const mat4& vp)
{
    // World position of windmill centre
    const float WM_X  = 24.0f;
    const float WM_Z  = -25.0f;   // hole centre z

    // ── Base blocks (either side of the gap, spanning hole inner width) ───────
    // Gap: z in [WM_Z-0.60, WM_Z+0.60] — ball passes through here
    const float GAP   = 0.60f;
    const float B_ZN  = -26.72f;   // north inner-wall face (H11_ZN + WTH)
    const float B_ZS  = -23.28f;   // south inner-wall face
    const float B_XH  = 1.0f;      // half-depth in x
    const float B_Y   = 0.50f;     // base block height

    // North base block
    { float cz = (B_ZN + WM_Z - GAP) * 0.5f;
      float sz = (WM_Z - GAP) - B_ZN;
      mat4 m = glm::translate(mat4(1), {WM_X, B_Y * 0.5f, cz});
      m = glm::scale(m, {B_XH * 2.f, B_Y, sz});
      draw(mBox, m, vp, 27); }

    // South base block
    { float cz = (WM_Z + GAP + B_ZS) * 0.5f;
      float sz = B_ZS - (WM_Z + GAP);
      mat4 m = glm::translate(mat4(1), {WM_X, B_Y * 0.5f, cz});
      m = glm::scale(m, {B_XH * 2.f, B_Y, sz});
      draw(mBox, m, vp, 27); }

    // ── Tapered tower (4 steps, dark → light brown) ───────────────────────────
    // Each step: centre=(WM_X, y_c, WM_Z), scale=(x_depth, height, z_width)
    const float ST[][5] = {
        // x_half, z_half, y0,   y1,  surf
        { 1.00f, 1.72f, 0.50f, 1.00f, 28 },
        { 0.85f, 1.45f, 1.00f, 1.50f, 29 },
        { 0.65f, 1.06f, 1.50f, 2.00f, 29 },
        { 0.50f, 0.72f, 2.00f, 2.50f, 30 },
    };
    for (auto& s : ST) {
        mat4 m = glm::translate(mat4(1), {WM_X, (s[2]+s[3])*0.5f, WM_Z});
        m = glm::scale(m, {s[0]*2.f, s[3]-s[2], s[1]*2.f});
        draw(mBox, m, vp, (int)s[4]);
    }

    // Roof cap
    { mat4 m = glm::translate(mat4(1), {WM_X, 2.80f, WM_Z});
      m = glm::scale(m, {0.70f, 0.60f, 0.80f});
      draw(mBox, m, vp, 30); }

    // ── Axle — horizontal cylinder pointing west (−X) from tower face ────────
    // Spans x = [WM_X-1.25 to WM_X-0.65], y=2.0, z=WM_Z
    { mat4 m = glm::translate(mat4(1), {WM_X - 0.95f, 2.0f, WM_Z});
      m = glm::rotate(m, 1.5707963f, {0.f, 0.f, 1.f});  // Y-cyl → X-cyl
      m = glm::scale(m, {0.10f, 0.60f, 0.10f});
      draw(mCylinder, m, vp, 16); }   // grey pole colour

    // ── Spinning rotor — 4 arms in ZY-plane around X axis ────────────────────
    // Pivot at (WM_X-1.25, 2.0, WM_Z), spin around X axis
    const float bD  = 0.08f;    // blade thickness in x
    const float bW  = 0.32f;    // blade width (perpendicular arm width)
    const float bL  = 1.72f;    // arm half-span (tip to tip / 2)

    mat4 rotor = glm::translate(mat4(1), {WM_X - 1.25f, 2.0f, WM_Z});
    rotor = glm::rotate(rotor, windmillAngle, {1.f, 0.f, 0.f});

    // +Y arm
    { mat4 m = glm::translate(rotor, {0.f, bL * 0.5f + 0.10f, 0.f});
      m = glm::scale(m, {bD, bL, bW});
      draw(mBox, m, vp, 6); }
    // -Y arm
    { mat4 m = glm::translate(rotor, {0.f, -(bL * 0.5f + 0.10f), 0.f});
      m = glm::scale(m, {bD, bL, bW});
      draw(mBox, m, vp, 6); }
    // +Z arm
    { mat4 m = glm::translate(rotor, {0.f, 0.f,  bL * 0.5f + 0.10f});
      m = glm::scale(m, {bD, bW, bL});
      draw(mBox, m, vp, 6); }
    // -Z arm
    { mat4 m = glm::translate(rotor, {0.f, 0.f, -(bL * 0.5f + 0.10f)});
      m = glm::scale(m, {bD, bW, bL});
      draw(mBox, m, vp, 6); }
}

// ── Decorative bridge near east edge, crossing the river ─────────────────────
// Elevated wooden deck on stilts with post-and-rail fencing and stairs at
// both ends. Not a playable hole — purely scenic.
static void renderBridge(const mat4& vp)
{
    const float BX_W   = 39.0f;
    const float BX_E   = 41.0f;
    const float BX_C   = (BX_W + BX_E) * 0.5f;   // 42.0
    const float BZ_N   = -26.0f;   // shifted 1 unit south
    const float BZ_S   = -16.0f;
    const float BZ_C   = (BZ_N + BZ_S) * 0.5f;   // -20.0
    const float BZ_LEN = BZ_S - BZ_N;             // 10.0
    const float BW     = BX_E - BX_W;             //  2.0
    const float H      = 1.5f;                    // deck elevation
    const float POST_R = 0.13f;
    const float STEP_H = 0.30f;
    const float STEP_D = 0.40f;
    const int   NSTEPS = 5;                       // 5 × 0.3 = 1.5 = H

    // ── Elevated deck ─────────────────────────────────────────────────────────
    { mat4 m = glm::translate(mat4(1), {BX_C, H, BZ_C});
      m = glm::scale(m, {BW, 1.f, BZ_LEN});
      draw(mQuad, m, vp, 6); }

    // ── Support stilts (barely below ground → deck) ──────────────────────────
    // mCylinder goes from y=0 to y=1, so translate sets BOTTOM, scale sets HEIGHT
    const float SINK = 0.1f;   // barely penetrate ground/river
    auto stilt = [&](float px, float pz) {
        mat4 m = glm::translate(mat4(1), {px, -SINK, pz});
        m = glm::scale(m, {POST_R * 2.f, H + SINK, POST_R * 2.f});
        draw(mCylinder, m, vp, 6);
    };
    for (float pz = BZ_N; pz <= BZ_S + 0.01f; pz += 2.5f) {
        stilt(BX_W, pz);
        stilt(BX_E, pz);
    }

    // ── Railing posts (sit on top of deck surface) ───────────────────────────
    const float RP_H = 0.65f;
    auto rpost = [&](float px, float pz) {
        mat4 m = glm::translate(mat4(1), {px, H, pz});   // bottom at deck level
        m = glm::scale(m, {0.10f, RP_H, 0.10f});
        draw(mCylinder, m, vp, 6);
    };
    for (float pz = BZ_N; pz <= BZ_S + 0.01f; pz += 1.25f) {
        rpost(BX_W, pz);
        rpost(BX_E, pz);
    }

    // ── Horizontal rails — 2 per side ────────────────────────────────────────
    const float RAIL_T = 0.07f;
    for (int side = 0; side < 2; side++) {
        float px = (side == 0) ? BX_W : BX_E;
        { mat4 m = glm::translate(mat4(1), {px, H + RP_H * 0.30f, BZ_C});
          m = glm::scale(m, {RAIL_T, RAIL_T, BZ_LEN});
          draw(mBox, m, vp, 6); }
        { mat4 m = glm::translate(mat4(1), {px, H + RP_H * 0.85f, BZ_C});
          m = glm::scale(m, {RAIL_T, RAIL_T, BZ_LEN});
          draw(mBox, m, vp, 6); }
    }

    // ── South stairs — lowest step farthest from bridge, highest adjacent to deck
    // (approach from south: first step low, last step at deck height)
    for (int s = 0; s < NSTEPS; s++) {
        float zc = BZ_S + (NSTEPS - s - 0.5f) * STEP_D;  // s=0 farthest, s=4 closest
        float h  = (s + 1) * STEP_H;                       // s=0 lowest, s=4 tallest
        mat4 m = glm::translate(mat4(1), {BX_C, h * 0.5f, zc});
        m = glm::scale(m, {BW, h, STEP_D});
        draw(mBox, m, vp, 6);
    }

    // ── North stairs — facing away from south stairs (lowest at far north end)
    for (int s = 0; s < NSTEPS; s++) {
        float zc = BZ_N - (NSTEPS - s - 0.5f) * STEP_D;  // s=0 farthest north, s=4 closest
        float h  = (s + 1) * STEP_H;
        mat4 m = glm::translate(mat4(1), {BX_C, h * 0.5f, zc});
        m = glm::scale(m, {BW, h, STEP_D});
        draw(mBox, m, vp, 6);
    }
}

static void drawRestaurant(const mat4& vp, float yOff = 0.f){
    float cx = 0.f, cz = 0.f;
    float bw = 10.f, bd = 8.f, bh = 3.5f;

    // Floor
    { mat4 m = glm::translate(mat4(1), {cx, yOff+0.01f, cz}); m = glm::scale(m, {bw, 1, bd}); draw(mQuad, m, vp, 9); }

    // Walls
    { mat4 m = glm::translate(mat4(1), {cx, yOff+bh*.5f, cz-bd*.5f}); m = glm::scale(m, {bw, bh, .28f}); draw(mBox, m, vp, 8); }
    { mat4 m = glm::translate(mat4(1), {cx, yOff+bh*.5f, cz+bd*.5f}); m = glm::scale(m, {bw, bh, .28f}); draw(mBox, m, vp, 8); }
    { mat4 m = glm::translate(mat4(1), {cx-bw*.5f, yOff+bh*.5f, cz}); m = glm::scale(m, {.28f, bh, bd}); draw(mBox, m, vp, 8); }
    { mat4 m = glm::translate(mat4(1), {cx+bw*.5f, yOff+bh*.5f, cz}); m = glm::scale(m, {.28f, bh, bd}); draw(mBox, m, vp, 8); }

    auto windowFrontBack = [&](float x, float z, float dir){
        float y = yOff + bh * 0.6f;
        float off = 0.08f * dir;
        { mat4 m = glm::translate(mat4(1), {x, y, z}); m = glm::scale(m, {1.5f, 1.2f, 0.05f}); draw(mBox, m, vp, 15); }
        { mat4 m = glm::translate(mat4(1), {x, y+0.65f, z+off}); m = glm::scale(m, {1.8f, 0.12f, 0.10f}); draw(mBox, m, vp, 13); }
        { mat4 m = glm::translate(mat4(1), {x, y-0.65f, z+off}); m = glm::scale(m, {1.8f, 0.12f, 0.10f}); draw(mBox, m, vp, 13); }
        { mat4 m = glm::translate(mat4(1), {x-0.85f, y, z+off}); m = glm::scale(m, {0.12f, 1.4f, 0.10f}); draw(mBox, m, vp, 13); }
        { mat4 m = glm::translate(mat4(1), {x+0.85f, y, z+off}); m = glm::scale(m, {0.12f, 1.4f, 0.10f}); draw(mBox, m, vp, 13); }
        { mat4 m = glm::translate(mat4(1), {x, y, z+off}); m = glm::scale(m, {0.12f, 1.3f, 0.12f}); draw(mBox, m, vp, 13); }
        { mat4 m = glm::translate(mat4(1), {x, y, z+off}); m = glm::scale(m, {1.6f, 0.12f, 0.12f}); draw(mBox, m, vp, 13); }
    };

    auto windowSides = [&](float x, float z, float dir){
        float y = yOff + bh * 0.6f;
        float off = 0.08f * dir;
        { mat4 m = glm::translate(mat4(1), {x, y, z}); m = glm::scale(m, {0.05f, 1.2f, 1.5f}); draw(mBox, m, vp, 15); }
        { mat4 m = glm::translate(mat4(1), {x+off, y+0.65f, z}); m = glm::scale(m, {0.10f, 0.12f, 1.8f}); draw(mBox, m, vp, 13); }
        { mat4 m = glm::translate(mat4(1), {x+off, y-0.65f, z}); m = glm::scale(m, {0.10f, 0.12f, 1.8f}); draw(mBox, m, vp, 13); }
        { mat4 m = glm::translate(mat4(1), {x+off, y, z-0.85f}); m = glm::scale(m, {0.10f, 1.4f, 0.12f}); draw(mBox, m, vp, 13); }
        { mat4 m = glm::translate(mat4(1), {x+off, y, z+0.85f}); m = glm::scale(m, {0.10f, 1.4f, 0.12f}); draw(mBox, m, vp, 13); }
        { mat4 m = glm::translate(mat4(1), {x+off, y, z}); m = glm::scale(m, {0.12f, 1.3f, 0.12f}); draw(mBox, m, vp, 13); }
        { mat4 m = glm::translate(mat4(1), {x+off, y, z}); m = glm::scale(m, {0.12f, 0.12f, 1.6f}); draw(mBox, m, vp, 13); }
    };

    windowFrontBack(cx-2.8f, cz+bd*0.5f+0.16f,  1.f);
    windowFrontBack(cx+2.8f, cz+bd*0.5f+0.16f,  1.f);
    windowFrontBack(cx-2.8f, cz-bd*0.5f-0.16f, -1.f);
    windowFrontBack(cx+2.8f, cz-bd*0.5f-0.16f, -1.f);

    windowSides(cx-bw*0.5f-0.16f, cz-2.f, -1.f);
    windowSides(cx-bw*0.5f-0.16f, cz+2.f, -1.f);
    windowSides(cx+bw*0.5f+0.16f, cz-2.f,  1.f);
    windowSides(cx+bw*0.5f+0.16f, cz+2.f,  1.f);

    // Double doors (south and north faces)
    { mat4 m = glm::translate(mat4(1), {cx, yOff+1.35f, cz+bd*0.5f+0.18f}); m = glm::scale(m, {2.2f, 2.7f, 0.14f}); draw(mBox, m, vp, 13); }
    { mat4 m = glm::translate(mat4(1), {cx, yOff+1.35f, cz+bd*0.5f+0.28f}); m = glm::scale(m, {0.08f, 2.5f, 0.08f}); draw(mBox, m, vp, 8); }
    { mat4 m = glm::translate(mat4(1), {cx, yOff+1.35f, cz-bd*0.5f-0.18f}); m = glm::scale(m, {2.2f, 2.7f, 0.14f}); draw(mBox, m, vp, 13); }
    { mat4 m = glm::translate(mat4(1), {cx, yOff+1.35f, cz-bd*0.5f-0.28f}); m = glm::scale(m, {0.08f, 2.5f, 0.08f}); draw(mBox, m, vp, 8); }

    float roofY = yOff + bh + 0.15f;
    float roofW = bw + 1.8f;
    float roofD = bd + 1.8f;
    float roofH = 2.0f;

    // Eave base
    { mat4 m = glm::translate(mat4(1), {cx, yOff+bh+0.03f, cz}); m = glm::scale(m, {roofW, 0.18f, roofD}); draw(mBox, m, vp, 6); }

    // Left slope panel
    { mat4 m = glm::translate(mat4(1), {cx - roofW*0.255f, roofY + roofH*0.36f, cz});
      m = glm::rotate(m, glm::radians(28.0f), vec3(0, 0, 1));
      m = glm::scale(m, {roofW*0.60f, 0.22f, roofD}); draw(mBox, m, vp, 6); }

    // Right slope panel
    { mat4 m = glm::translate(mat4(1), {cx + roofW*0.255f, roofY + roofH*0.36f, cz});
      m = glm::rotate(m, glm::radians(-28.0f), vec3(0, 0, 1));
      m = glm::scale(m, {roofW*0.60f, 0.22f, roofD}); draw(mBox, m, vp, 6); }

    // Battens
    for(int i = -5; i <= 5; ++i){
        float z = cz + i * (roofD / 11.0f);
        { mat4 m = glm::translate(mat4(1), {cx - roofW*0.255f, roofY + roofH*0.48f, z});
          m = glm::rotate(m, glm::radians(28.0f), vec3(0, 0, 1));
          m = glm::scale(m, {roofW*0.58f, 0.045f, 0.045f}); draw(mBox, m, vp, 7); }
        { mat4 m = glm::translate(mat4(1), {cx + roofW*0.255f, roofY + roofH*0.48f, z});
          m = glm::rotate(m, glm::radians(-28.0f), vec3(0, 0, 1));
          m = glm::scale(m, {roofW*0.58f, 0.045f, 0.045f}); draw(mBox, m, vp, 7); }
    }

    // Ridge beam
    { mat4 m = glm::translate(mat4(1), {cx, roofY + roofH*0.78f, cz});
      m = glm::scale(m, {0.32f, 0.32f, roofD+0.35f}); draw(mBox, m, vp, 7); }

    // Gable end pieces (front)
    { mat4 m = glm::translate(mat4(1), {cx - 1.55f, yOff+bh+0.75f, cz + roofD*0.505f});
      m = glm::rotate(m, glm::radians(28.0f), vec3(0, 0, 1));
      m = glm::scale(m, {3.6f, 0.12f, 0.16f}); draw(mBox, m, vp, 7); }
    { mat4 m = glm::translate(mat4(1), {cx + 1.55f, yOff+bh+0.75f, cz + roofD*0.505f});
      m = glm::rotate(m, glm::radians(-28.0f), vec3(0, 0, 1));
      m = glm::scale(m, {3.6f, 0.12f, 0.16f}); draw(mBox, m, vp, 7); }

    // Gable end pieces (back)
    { mat4 m = glm::translate(mat4(1), {cx - 1.55f, yOff+bh+0.75f, cz - roofD*0.505f});
      m = glm::rotate(m, glm::radians(28.0f), vec3(0, 0, 1));
      m = glm::scale(m, {3.6f, 0.12f, 0.16f}); draw(mBox, m, vp, 7); }
    { mat4 m = glm::translate(mat4(1), {cx + 1.55f, yOff+bh+0.75f, cz - roofD*0.505f});
      m = glm::rotate(m, glm::radians(-28.0f), vec3(0, 0, 1));
      m = glm::scale(m, {3.6f, 0.12f, 0.16f}); draw(mBox, m, vp, 7); }

    // Sign board (front and back) — surf 34 cream/ivory
    { mat4 m = glm::translate(mat4(1), {cx, yOff+bh+3.15f, cz + 0.16f});
      m = glm::scale(m, {5.0f, 1.35f, 0.12f}); draw(mBox, m, vp, 34); }
    { mat4 m = glm::translate(mat4(1), {cx, yOff+bh+3.15f, cz - 0.16f});
      m = glm::scale(m, {5.0f, 1.35f, 0.12f}); draw(mBox, m, vp, 34); }

    // Sign posts
    { mat4 m = glm::translate(mat4(1), {cx - 2.0f, yOff+bh+2.2f, cz});
      m = glm::scale(m, {0.11f, 2.2f, 0.11f}); draw(mBox, m, vp, 13); }
    { mat4 m = glm::translate(mat4(1), {cx + 2.0f, yOff+bh+2.2f, cz});
      m = glm::scale(m, {0.11f, 2.2f, 0.11f}); draw(mBox, m, vp, 13); }
}

static Mesh makeSkyboxMesh(){
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
    Mesh m; m.vao=vao; m.vbo=vbo; m.ebo=0; m.count=36;
    return m;
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main(){
    if(!glfwInit()){ puts("glfwInit failed"); return 1; }
    glfwWindowHint(GLFW_SAMPLES,4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif
    gWin = glfwCreateWindow(WIN_W,WIN_H,"COS344 HA - Mini Golf (u14439141)",NULL,NULL);
    if(!gWin){ glfwTerminate(); puts("Window failed"); return 1; }
    glfwMakeContextCurrent(gWin);
    glewExperimental=true;
    if(glewInit()!=GLEW_OK){ glfwTerminate(); puts("GLEW failed"); return 1; }

    glfwSetKeyCallback(gWin,cbKey);
    glfwSetMouseButtonCallback(gWin,cbMouseBtn);
    glfwSetCursorPosCallback(gWin,cbMouseMove);
    glfwSetScrollCallback(gWin,cbScroll);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.01f,0.02f,0.06f,1);

    gProg   = LoadShaders("golf_vert.glsl","golf_frag.glsl");
    skyProg = LoadShaders("sky_vert.glsl", "sky_frag.glsl");
    skyTex  = loadTexture("day_sky.png");

    mQuad     = makeQuad();
    mBox      = makeBox();
    mSphere   = makeSphere();
    mCylinder = makeCylinder();
    mTorus    = makeTorus(1.f,0.05f,28,10);
    mTrap     = makeTrapezoid();
    mCircle   = makeCircle();
    mSkybox  = makeSkyboxMesh();
    mVQuad        = makeVQuad();
    mLeafVQuad    = makeLeafVQuad();
    mRockyBoulder = makeRockyBoulder();
    mTerrainHills = makeHillTerrain(33.f, 38.f, 16.f, 14.f, 32, 28);
    mTaperedCyl        = makeTaperedCylinder(0.18f, 0.10f, 18);
    mThickCyl          = makeTaperedCylinder(0.32f, 0.18f, 18);
    mRestaurantPlateau = makeRestaurantPlateau();
    mTerrainWest       = makeWestHillTerrain();

    // ── River + pond meshes ───────────────────────────────────────────────────
    {
        // Right edge of trapezoid at z=-22: x = 40 + (55-22)/11 = 43.0 exactly.
        // Exit bends west to be roughly parallel with the bottom edge (z=55),
        // then stops flush at the lower-left corner area.
        vector<vec3> pts = {
            {43.f, 0,  -22.f},   // entry: flush with right course edge
            {37.f, 0,  -18.f},
            {29.f, 0,  -11.f},
            {22.f, 0,   -4.f},
            {17.f, 0,    3.f},   // right of restaurant
            {15.5f,0,   10.f},   // pond transition start
            {14.f, 0,   16.f},   // pond centre
            {13.f, 0,   23.f},   // pond transition end
            { 9.f, 0,   31.f},
            { 2.f, 0,   37.f},
            {-5.f, 0,   41.f},   // exit-bend starts here — turning strongly westward
            {-11.f,0,   44.f},
            {-19.f,0,  46.5f},
            {-28.f,0,  48.5f},   // ~77° west — near-parallel to bottom edge
            {-36.f,0,   50.f},   // ~79° west
            {-42.f,0,   51.f},   // ~82° west — nearly parallel
            {-49.f,0,  51.5f},   // ~86° west — exits west edge flush, parallel to bottom
        };
        vector<float> ww = {3.5f,3.5f,3.5f,3.5f,3.5f, 8.f,16.f,8.f, 3.5f,3.5f,3.5f,3.5f,3.5f,3.5f,3.5f,3.5f,3.5f};
        vector<float> bw; for(float w : ww) bw.push_back(w + 4.f);
        mRiverBank = makeRiverStrip(pts, bw, 0.001f);  // dirt banks — slightly above grass
        mRiver     = makeRiverStrip(pts, ww, 0.003f);  // water — above bank so it's visible
    }

    // ── Load textures ──────────────────────────────────────────────────────────
    for(int i=0; i<8; i++){
        char buf[128];
        snprintf(buf,sizeof(buf),"textures/rocks/rock%d.bmp",i+1);
        texRock[i] = loadTexture(buf);
    }
    texBark = loadTexture("textures/rocks/wood4.bmp");
    const char* crownPaths[3]={
        "textures/tropical/t_palm_shrub1/palms1.png",
        "textures/tropical/t_palm_shrub2/palms2.png",
        "textures/tropical/t_palm_shrub3/palms3.png",
    };
    for(int i=0; i<3; i++) texPalmCrown[i] = loadTexture(crownPaths[i]);
    const char* weedPaths[3]={
        "textures/tropical/t_trop_weed1/tropw1.png",
        "textures/tropical/t_trop_weed6/tropw6.png",
        "textures/tropical/t_trop_weed7/tropw7.png",
    };
    for(int i=0; i<3; i++) texWeed[i] = loadTexture(weedPaths[i]);
    const char* shrubPaths[2]={
        "textures/tropical/t_trop_shrub1/trops1.png",
        "textures/tropical/t_trop_shrub2/trops2.png",
    };
    for(int i=0; i<2; i++) texShrub[i] = loadTexture(shrubPaths[i]);
    texConcrete = loadTexture("textures/rocks/floor1.bmp");
    texAgave        = loadTexture("textures/tropical/t_agave1/agava1.png");
    texFlax         = loadTexture("textures/tropical/t_flax1_green/flax1g.png");
    texLowPalmCrown = loadTexture("textures/tropical/t_lowpalm1/lpalm1.png");

    gHoles[1]  = new Hole1(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[2]  = new Hole2(&mQuad, &mBox, &mCylinder, &mTorus, &mSphere);
    gHoles[3]  = new Hole3(&mQuad, &mBox, &mCylinder, &mTorus, &mCircle);
    gHoles[4]  = new Hole4(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[5]  = new Hole5(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[6]  = new Hole6(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[7]  = new Hole7(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[8]  = new Hole8(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[9]  = new Hole9(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[10] = new Hole10(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[11] = new Hole11(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[12] = new Hole12(&mQuad, &mBox, &mCylinder, &mTorus);

    double prev = glfwGetTime();

    while(!glfwWindowShouldClose(gWin)){
        double now = glfwGetTime();
        float  dt  = (float)(now-prev);
        prev = now;

        // Time of day
        timeOfDay = fmodf(timeOfDay + todSpeed*dt, 1.f);

        // Propeller spin
        propAngle = fmodf(propAngle + dt * 18.f, 2*PI);

        // Hole 11 windmill rotor
        windmillAngle = fmodf(windmillAngle + dt * 1.8f, 2*PI);

        // Ball update
        ball.update(dt);

        // Terrain physics (groundY + slope force delegated to each hole)
        if(ball.active){
            if(gHoles[gCurrentHole]){
                ball.pos.y = gHoles[gCurrentHole]->groundY(ball.pos);
                vec3 tf = gHoles[gCurrentHole]->terrainForce(ball.pos);
                if(glm::length(tf) > 0.001f){
                    ball.vel += tf * dt;
                    if(glm::length(ball.vel) > 0.01f) ball.moving = true;
                }
            } else {
                ball.pos.y = BALL_R;
            }
        }

        // Hole completion → advance to next hole
        if(ball.active && !ball.inHole && ball.nearCup()){
            ball.inHole = true;
            ball.moving = false;
            printf("Hole %d: %d stroke%s\n",
                   gCurrentHole, ball.strokes, ball.strokes==1?"":"s");
            if(gCurrentHole < 12){
                gCurrentHole++;
                ball.pos = gHoles[gCurrentHole] ? gHoles[gCurrentHole]->getTeePos()
                                                : vec3(0.f, BALL_R, 0.f);
                ball.vel = {0,0,0};
                ball.active = true; ball.moving = false;
                ball.inHole = false; ball.strokes = 0;
                aimMode = false;
            } else {
                ball.active = false;
                printf("Course complete!\n");
            }
        }

        // ── Camera / drone movement ──
        float spd = cam.spd * dt;

        if(droneView){
            // WASD + QE fly the drone
            vec3 fw = {sinf(drone.yaw)*cosf(drone.pitch),
                       sinf(drone.pitch),
                      -cosf(drone.yaw)*cosf(drone.pitch)};
            vec3 rt = glm::normalize(glm::cross(fw,{0,1,0}));
            if(glfwGetKey(gWin,GLFW_KEY_W)==GLFW_PRESS) drone.pos += fw*spd;
            if(glfwGetKey(gWin,GLFW_KEY_S)==GLFW_PRESS) drone.pos -= fw*spd;
            if(glfwGetKey(gWin,GLFW_KEY_A)==GLFW_PRESS) drone.pos -= rt*spd;
            if(glfwGetKey(gWin,GLFW_KEY_D)==GLFW_PRESS) drone.pos += rt*spd;
            if(glfwGetKey(gWin,GLFW_KEY_Q)==GLFW_PRESS) drone.pos.y -= spd;
            if(glfwGetKey(gWin,GLFW_KEY_E)==GLFW_PRESS) drone.pos.y += spd;
            // Lock camera to drone
            cam.pos   = drone.pos;
            cam.yaw   = drone.yaw;
            cam.pitch = drone.pitch;
        } else {
            // WASD + QE move external observer freely
            vec3 fw = cam.fwd();
            vec3 rt = glm::normalize(glm::cross(fw,{0,1,0}));
            if(glfwGetKey(gWin,GLFW_KEY_W)==GLFW_PRESS) cam.pos += fw*spd;
            if(glfwGetKey(gWin,GLFW_KEY_S)==GLFW_PRESS) cam.pos -= fw*spd;
            if(glfwGetKey(gWin,GLFW_KEY_A)==GLFW_PRESS) cam.pos -= rt*spd;
            if(glfwGetKey(gWin,GLFW_KEY_D)==GLFW_PRESS) cam.pos += rt*spd;
            if(glfwGetKey(gWin,GLFW_KEY_Q)==GLFW_PRESS) cam.pos.y -= spd;
            if(glfwGetKey(gWin,GLFW_KEY_E)==GLFW_PRESS) cam.pos.y += spd;
        }

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        float asp = (float)WIN_W/WIN_H;
        mat4 view = cam.view();
        mat4 proj = cam.proj(asp);
        mat4 vp   = proj*view;

        // ── Skybox ──
        glDepthFunc(GL_LEQUAL); glDepthMask(GL_FALSE);
        glUseProgram(skyProg);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skyTex);
        glUniform1i(glGetUniformLocation(skyProg,"skyTex"), 0);
        mat4 skyVP = proj*mat4(mat3(view));
        glUniformMatrix4fv(glGetUniformLocation(skyProg,"uVP"),1,GL_FALSE,glm::value_ptr(skyVP));
        glUniform1f(glGetUniformLocation(skyProg,"uTimeOfDay"),timeOfDay);
        glBindVertexArray(mSkybox.vao);
        glDrawArrays(GL_TRIANGLES,0,36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE); glDepthFunc(GL_LESS);

        // ── Scene ──
        glUseProgram(gProg);
        setUniforms((float)now, vp);

        draw(mTrap, mat4(1), vp, 2);

        for(int i = 1; i <= 12; i++)
            if(gHoles[i]) gHoles[i]->render(vp);

        drawWindmill(vp);
        renderBridge(vp);

        // Restaurant plateau — gradual slope rising to PLAT_H around restaurant area
        draw(mRestaurantPlateau, mat4(1), vp, 2);

        drawRestaurant(vp, PLAT_H);

        // Concrete slabs on the plateau top (bw=10, bd=8 → walls at x=±5, z=±4)
        const float sy = PLAT_H + 0.005f;  // slab y: just above plateau top
        // Front slab: z=4..8
        { mat4 m=glm::translate(mat4(1),{0.f,sy, 6.f}); m=glm::scale(m,{16.f,1.f,4.f}); drawWithTex(mQuad,m,vp,8,texConcrete); }
        // Back slab: z=-4..-7
        { mat4 m=glm::translate(mat4(1),{0.f,sy,-5.5f}); m=glm::scale(m,{16.f,1.f,3.f}); drawWithTex(mQuad,m,vp,8,texConcrete); }
        // Left slab: x=-5..-8
        { mat4 m=glm::translate(mat4(1),{-6.5f,sy,0.f}); m=glm::scale(m,{3.f,1.f, 8.f}); drawWithTex(mQuad,m,vp,8,texConcrete); }
        // Right slab: x=5..9
        { mat4 m=glm::translate(mat4(1),{ 7.f, sy,0.f}); m=glm::scale(m,{4.f,1.f, 8.f}); drawWithTex(mQuad,m,vp,8,texConcrete); }

        // ── Seating area (all on plateau top) ───────────────────────────────────
        const float ty = PLAT_H;
        drawTable({-3.5f, ty, 6.0f}, vp);
        drawChair({-4.5f, ty, 6.0f},  1.57f, vp);
        drawChair({-2.5f, ty, 6.0f}, -1.57f, vp);
        drawChair({-3.5f, ty, 7.0f},  3.14f, vp);
        drawChair({-3.5f, ty, 5.0f},  0.0f,  vp);

        drawTable({ 3.5f, ty, 6.0f}, vp);
        drawChair({ 2.5f, ty, 6.0f},  1.57f, vp);
        drawChair({ 4.5f, ty, 6.0f}, -1.57f, vp);
        drawChair({ 3.5f, ty, 7.0f},  3.14f, vp);
        drawChair({ 3.5f, ty, 5.0f},  0.0f,  vp);

        drawTable({7.5f, ty, 1.5f}, vp);
        drawChair({6.5f, ty, 1.5f},  1.57f, vp);
        drawChair({8.5f, ty, 1.5f}, -1.57f, vp);
        drawChair({7.5f, ty, 2.5f},  3.14f, vp);
        drawChair({7.5f, ty, 0.5f},  0.0f,  vp);

        // ── Corner gardens on plateau top ────────────────────────────────────────
        const float gy = ty + 0.02f;
        { mat4 m=glm::translate(mat4(1),{-6.5f,gy, 7.5f}); m=glm::scale(m,{1.6f,1.f,1.6f}); draw(mQuad,m,vp,23); }
        drawBoulder({-6.5f, ty+0.4f*0.35f, 7.5f}, 0.40f, texRock[2], vp);
        drawBillboard({-6.8f, ty, 7.2f}, 0.65f, 1.0f, texWeed[0], vp);
        drawBillboard({-6.2f, ty, 7.8f}, 0.55f, 0.85f, texWeed[1], vp);

        { mat4 m=glm::translate(mat4(1),{ 6.5f,gy, 7.5f}); m=glm::scale(m,{1.6f,1.f,1.6f}); draw(mQuad,m,vp,23); }
        drawBoulder({ 6.5f, ty+0.35f*0.35f, 7.5f}, 0.35f, texRock[3], vp);
        drawBillboard({ 6.2f, ty, 7.2f}, 0.60f, 0.90f, texWeed[0], vp);

        { mat4 m=glm::translate(mat4(1),{ 8.5f,gy, 3.5f}); m=glm::scale(m,{1.6f,1.f,1.6f}); draw(mQuad,m,vp,23); }
        drawBoulder({ 8.5f, ty+0.45f*0.35f, 3.5f}, 0.45f, texRock[4], vp);
        drawBillboard({ 8.8f, ty, 3.2f}, 0.60f, 0.95f, texWeed[1], vp);
        drawBillboard({ 8.2f, ty, 3.8f}, 0.50f, 0.80f, texWeed[0], vp);

        { mat4 m=glm::translate(mat4(1),{ 8.5f,gy,-3.5f}); m=glm::scale(m,{1.6f,1.f,1.6f}); draw(mQuad,m,vp,23); }
        drawBoulder({ 8.5f, ty+0.38f*0.35f,-3.5f}, 0.38f, texRock[5], vp);
        drawBillboard({ 8.8f, ty,-3.8f}, 0.55f, 0.85f, texWeed[1], vp);

        // Lamp posts
        for(int i = 0; i < LAMP_COUNT; i++)
            drawLampPost(LAMP_POSITIONS[i], vp);

        // Pond island — drawn before water so depth test blocks water over the island
        { mat4 m = glm::translate(mat4(1), {14.f, -0.3f, 16.f});
          m = glm::scale(m, {3.5f, 0.85f, 3.5f});
          draw(mSphere, m, vp, 26); }  // dark earth dome; island top ≈ y=0.55

        // River + pond (drawn after island so water fills around the dome)
        draw(mRiverBank, mat4(1), vp, 23);  // dirt/soil banks
        draw(mRiver,     mat4(1), vp,  5);  // animated water

        // Island vegetation — boulders, palm and plants grouped on the small hill
        // Two large feature boulders on the island slopes
        drawBoulder({13.0f, 0.35f, 14.5f}, 0.70f, texRock[2], vp);
        drawBoulder({15.5f, 0.30f, 17.5f}, 0.65f, texRock[5], vp);
        // Small rocks
        drawBoulder({15.8f, 0.45f, 15.2f}, 0.30f, texRock[1], vp);
        drawBoulder({12.5f, 0.38f, 17.2f}, 0.25f, texRock[3], vp);
        drawPalmTree({14.f,  0.52f, 16.5f}, 4.5f, texBark, texPalmCrown[0], vp);
        drawBillboard({15.3f, 0.35f, 15.0f}, 0.45f, 0.75f, texWeed[0], vp);
        drawBillboard({12.8f, 0.28f, 17.5f}, 0.40f, 0.65f, texWeed[2], vp);
        drawBillboard({14.8f, 0.30f, 14.3f}, 0.50f, 0.70f, texWeed[1], vp);
        drawBillboard({13.5f, 0.38f, 17.9f}, 1.0f,  1.3f,  texShrub[0], vp);
        drawBillboard({14.5f, 0.42f, 14.8f}, 0.42f, 0.68f, texWeed[2], vp);
        drawBillboard({12.9f, 0.28f, 15.8f}, 0.38f, 0.60f, texWeed[0], vp);
        drawBillboard({15.1f, 0.32f, 16.9f}, 0.85f, 1.10f, texShrub[1], vp);

        // Large boulder marking where the plateau slope meets the river bank
        drawBoulder({12.5f, 0.30f, 4.5f}, 1.3f, texRock[6], vp);

        // ── River bank vegetation ─────────────────────────────────────────────
        // Positions pre-computed from perpendicular geometry at each river segment.
        // NE entry section (z=-18..-4)
        drawBoulder({35.0f,0.25f,-15.5f},0.50f,texRock[1],vp);
        drawBillboard({34.2f,0.f,-14.5f},0.9f,1.2f,texAgave,vp);
        drawBoulder({31.5f,0.22f,-17.5f},0.40f,texRock[3],vp);
        drawBillboard({30.5f,0.f,-16.5f},0.8f,1.3f,texFlax,vp);
        drawBoulder({27.0f,0.20f, -5.5f},0.45f,texRock[2],vp);
        drawBillboard({26.5f,0.f, -4.5f},1.0f,1.1f,texAgave,vp);
        drawBoulder({24.5f,0.22f,-11.0f},0.38f,texRock[4],vp);
        drawBillboard({23.8f,0.f,-10.0f},0.85f,1.2f,texFlax,vp);
        // Restaurant-side section (z=-4..10)
        drawBoulder({21.8f,0.20f,  1.5f},0.42f,texRock[5],vp);
        drawBillboard({21.0f,0.f,  2.5f},1.0f,1.3f,texAgave,vp);
        drawBoulder({17.5f,0.18f, -3.0f},0.35f,texRock[6],vp);
        drawBillboard({16.8f,0.f, -2.0f},0.9f,1.2f,texFlax,vp);
        drawBoulder({19.0f,0.20f,  7.8f},0.40f,texRock[0],vp);
        drawBillboard({19.8f,0.f,  8.8f},1.1f,1.4f,texAgave,vp);
        drawBoulder({13.5f,0.18f,  5.3f},0.38f,texRock[7],vp);
        drawBillboard({12.8f,0.f,  6.0f},0.8f,1.1f,texFlax,vp);
        // Pond area — north and south banks only (east bank near island already has vegetation)
        drawBoulder({20.4f,0.22f, 11.1f},0.50f,texRock[2],vp);
        drawBillboard({21.2f,0.f, 12.0f},1.0f,1.3f,texAgave,vp);
        drawBoulder({17.8f,0.20f, 24.3f},0.48f,texRock[4],vp);
        drawBillboard({18.5f,0.f, 25.3f},1.1f,1.2f,texFlax,vp);
        // South of pond section (z=23..37)
        drawBoulder({13.9f,0.22f, 27.8f},0.42f,texRock[1],vp);
        drawBillboard({14.7f,0.f, 28.8f},0.9f,1.3f,texAgave,vp);
        drawBoulder({ 8.2f,0.20f, 26.2f},0.38f,texRock[5],vp);
        drawBillboard({ 7.5f,0.f, 25.5f},0.85f,1.1f,texFlax,vp);
        drawBoulder({ 8.2f,0.22f, 32.6f},0.45f,texRock[3],vp);
        drawBillboard({ 8.9f,0.f, 33.5f},1.0f,1.2f,texAgave,vp);
        drawBoulder({ 2.8f,0.20f, 32.7f},0.40f,texRock[6],vp);
        drawBillboard({ 2.0f,0.f, 31.8f},0.9f,1.3f,texFlax,vp);
        // SW exit bend (z=37..51)
        drawBoulder({ 1.0f,0.22f, 40.7f},0.50f,texRock[0],vp);
        drawBillboard({ 0.0f,0.f, 41.5f},1.1f,1.2f,texAgave,vp);
        drawBoulder({-3.9f,0.20f, 37.3f},0.40f,texRock[2],vp);
        drawBillboard({-4.5f,0.f, 36.5f},0.85f,1.1f,texFlax,vp);
        drawBoulder({-5.4f,0.22f, 44.1f},0.45f,texRock[4],vp);
        drawBillboard({-4.6f,0.f, 45.0f},1.0f,1.3f,texAgave,vp);
        drawBoulder({-6.7f,0.20f, 37.9f},0.38f,texRock[7],vp);
        drawBillboard({-7.5f,0.f, 37.2f},0.9f,1.2f,texFlax,vp);
        drawBoulder({-9.7f,0.22f, 47.3f},0.42f,texRock[1],vp);
        drawBillboard({-8.8f,0.f, 48.1f},1.0f,1.3f,texAgave,vp);
        drawBoulder({-12.3f,0.20f,40.7f},0.40f,texRock[3],vp);
        drawBillboard({-13.0f,0.f,41.5f},0.85f,1.2f,texFlax,vp);
        drawBoulder({-18.1f,0.22f,49.9f},0.50f,texRock[5],vp);
        drawBillboard({-17.3f,0.f,50.7f},1.1f,1.3f,texAgave,vp);
        drawBoulder({-19.9f,0.20f,44.1f},0.38f,texRock[0],vp);
        drawBillboard({-20.7f,0.f,43.3f},0.9f,1.1f,texFlax,vp);
        drawBoulder({-26.4f,0.22f,46.7f},0.45f,texRock[6],vp);
        drawBillboard({-27.2f,0.f,45.8f},1.0f,1.3f,texAgave,vp);
        drawBoulder({-27.3f,0.20f,51.9f},0.42f,texRock[2],vp);
        drawBillboard({-26.5f,0.f,52.5f},0.9f,1.2f,texFlax,vp);
        drawBoulder({-29.1f,0.22f,48.6f},0.38f,texRock[4],vp);
        drawBillboard({-34.9f,0.f,48.6f},0.85f,1.1f,texAgave,vp);
        drawBoulder({-34.9f,0.20f,51.9f},0.42f,texRock[7],vp);
        drawBillboard({-29.0f,0.f,49.5f},1.0f,1.3f,texFlax,vp);

        // ── West-edge hill banks ──────────────────────────────────────────────
        draw(mTerrainWest, mat4(1), vp, 2);

        // Boulders at each hill peak and nearby secondary positions
        drawBoulder({-44.f,terrainHW(-44.f,-38.f)+1.8f*0.35f,-38.f},1.8f,texRock[0],vp);
        drawBoulder({-46.f,terrainHW(-46.f,-36.f)+1.4f*0.35f,-36.f},1.4f,texRock[2],vp);
        drawBoulder({-43.f,terrainHW(-43.f,-40.f)+1.1f*0.35f,-40.f},1.1f,texRock[5],vp);
        drawBoulder({-45.f,terrainHW(-45.f,-24.f)+1.5f*0.35f,-24.f},1.5f,texRock[1],vp);
        drawBoulder({-43.f,terrainHW(-43.f,-22.f)+1.0f*0.35f,-22.f},1.0f,texRock[4],vp);
        drawBoulder({-44.f,terrainHW(-44.f, -9.f)+1.7f*0.35f, -9.f},1.7f,texRock[3],vp);
        drawBoulder({-46.f,terrainHW(-46.f,-11.f)+1.2f*0.35f,-11.f},1.2f,texRock[6],vp);
        drawBoulder({-45.f,terrainHW(-45.f,  6.f)+1.4f*0.35f,  6.f},1.4f,texRock[2],vp);
        drawBoulder({-43.f,terrainHW(-43.f,  4.f)+1.0f*0.35f,  4.f},1.0f,texRock[7],vp);
        drawBoulder({-44.f,terrainHW(-44.f, 19.f)+1.6f*0.35f, 19.f},1.6f,texRock[0],vp);
        drawBoulder({-46.f,terrainHW(-46.f, 21.f)+1.1f*0.35f, 21.f},1.1f,texRock[5],vp);
        drawBoulder({-45.f,terrainHW(-45.f, 31.f)+1.5f*0.35f, 31.f},1.5f,texRock[1],vp);
        drawBoulder({-43.f,terrainHW(-43.f, 29.f)+1.0f*0.35f, 29.f},1.0f,texRock[3],vp);
        drawBoulder({-44.f,terrainHW(-44.f, 40.f)+1.8f*0.35f, 40.f},1.8f,texRock[4],vp);
        drawBoulder({-46.f,terrainHW(-46.f, 38.f)+1.3f*0.35f, 38.f},1.3f,texRock[6],vp);

        // Palm trees beside (not on top of) the boulders — offset 2-3 units in x and z
        drawPalmTree({-42.f,terrainHW(-42.f,-36.f),-36.f},6.0f,texBark,texPalmCrown[0],vp);
        drawPalmTree({-43.f,terrainHW(-43.f,-11.f),-11.f},5.5f,texBark,texPalmCrown[0],vp);
        drawPalmTree({-42.f,terrainHW(-42.f, 21.f), 21.f},6.5f,texBark,texPalmCrown[0],vp);
        drawPalmTree({-42.f,terrainHW(-42.f, 36.f), 36.f},5.8f,texBark,texPalmCrown[0],vp);
        drawPalmTree({-43.f,terrainHW(-43.f,-26.f),-26.f},5.0f,texBark,texPalmCrown[0],vp);
        drawPalmTree({-43.f,terrainHW(-43.f, 33.f), 33.f},5.5f,texBark,texPalmCrown[0],vp);

        // Weeds / shrubs scattered across west hills
        drawBillboard({-44.f,terrainHW(-44.f,-30.f),-30.f},1.5f,2.0f,texShrub[0],vp);
        drawBillboard({-43.f,terrainHW(-43.f,-15.f),-15.f},1.3f,1.8f,texWeed[1],vp);
        drawBillboard({-45.f,terrainHW(-45.f,  0.f),  0.f},1.4f,1.9f,texShrub[1],vp);
        drawBillboard({-44.f,terrainHW(-44.f, 12.f), 12.f},1.2f,1.6f,texWeed[0],vp);
        drawBillboard({-43.f,terrainHW(-43.f, 25.f), 25.f},1.5f,1.9f,texShrub[0],vp);
        drawBillboard({-45.f,terrainHW(-45.f, 36.f), 36.f},1.3f,1.7f,texWeed[2],vp);

        // ── Course-wide palm trees ────────────────────────────────────────────
        // Near restaurant / plateau approach
        drawPalmTree({-16.f,0.f,  8.f},5.5f,texBark,texPalmCrown[0],vp);
        drawPalmTree({-20.f,0.f, 15.f},6.0f,texBark,texPalmCrown[0],vp);
        drawPalmTree({-14.f,0.f, -6.f},5.0f,texBark,texPalmCrown[0],vp);
        // Between H1 and H2
        drawPalmTree({-28.f,0.f, 27.f},5.5f,texBark,texPalmCrown[0],vp);
        // Between H2 and H3
        drawPalmTree({-27.f,0.f,  8.f},5.0f,texBark,texPalmCrown[0],vp);
        // Between H3 and H4
        drawPalmTree({-27.f,0.f,-13.f},5.5f,texBark,texPalmCrown[0],vp);
        // Between H4 and H5
        drawPalmTree({-29.f,0.f,-27.f},5.0f,texBark,texPalmCrown[0],vp);
        drawBoulder({-35.0f,0.22f,-11.2f},0.42f,texRock[1],vp);
        drawBillboard({-35.8f,0.f,-12.0f},0.90f,1.20f,texAgave,vp);
        drawBillboard({-34.4f,0.f,-10.8f},0.80f,1.05f,texFlax,vp);
        drawLowPalmTree({-36.2f,0.f,-12.5f},2.8f,texPalmCrown[0],vp);
        drawBillboard({-34.5f,0.f,-12.8f},1.10f,1.40f,texShrub[1],vp);

        drawBoulder({-31.0f,0.22f,-29.6f},0.40f,texRock[5],vp);
        drawBillboard({-31.8f,0.f,-30.3f},0.95f,1.30f,texAgave,vp);
        drawBillboard({-30.3f,0.f,-28.9f},0.85f,1.10f,texFlax,vp);
        drawBillboard({-31.2f,0.f,-27.8f},1.20f,1.65f,texShrub[0],vp);
        drawLowPalmTree({-32.5f,0.f,-30.0f},3.0f,texPalmCrown[0],vp);

        drawBoulder({-31.0f,0.22f,-39.2f},0.40f,texRock[3],vp);
        drawBillboard({-30.2f,0.f,-39.8f},1.00f,1.30f,texFlax,vp);
        drawBillboard({-32.0f,0.f,-38.5f},0.85f,1.10f,texAgave,vp);
        drawBillboard({-31.8f,0.f,-40.7f},1.30f,1.55f,texShrub[1],vp);

        drawPalmTree({-22.f,0.f,-41.f},5.5f,texBark,texPalmCrown[0],vp);
        drawBoulder({-34.8f,0.22f,-32.2f},0.45f,texRock[2],vp);
        drawBillboard({-35.5f,0.f,-31.3f},0.95f,1.2f,texAgave,vp);
        drawBillboard({-33.8f,0.f,-33.0f},0.80f,1.1f,texFlax,vp);
        drawLowPalmTree({-33.2f,0.f,-35.0f},3.0f,texPalmCrown[0],vp);
        drawBillboard({-32.5f,0.f,-32.8f},1.20f,1.40f,texShrub[1],vp);
        drawPalmTree({  6.f,0.f,-33.f},5.0f,texBark,texPalmCrown[0],vp);
        drawPalmTree({ 18.f,0.f,-28.f},5.5f,texBark,texPalmCrown[0],vp);

        // ── Low palm trees (shorter, thicker trunk) scattered throughout ────────
        // Near restaurant entrance / path leading up
        drawLowPalmTree({-10.f, 0.f,   9.f}, 3.2f, texPalmCrown[0], vp);
        drawLowPalmTree({ -8.f, 0.f,  -8.f}, 2.8f, texPalmCrown[0], vp);
        drawLowPalmTree({  4.f, 0.f,   6.f}, 3.0f, texPalmCrown[0], vp);
        // Along hole 1 / 2 approach
        drawLowPalmTree({-24.f, 0.f,  35.f}, 2.6f, texPalmCrown[0], vp);
        drawLowPalmTree({-24.f, 0.f,  20.f}, 3.0f, texPalmCrown[0], vp);
        drawLowPalmTree({-25.f, 0.f,   5.f}, 2.8f, texPalmCrown[0], vp);
        // H3 / H4 area
        drawLowPalmTree({-24.f, 0.f, -10.f}, 3.2f, texPalmCrown[0], vp);
        drawLowPalmTree({-23.f, 0.f, -22.f}, 2.7f, texPalmCrown[0], vp);
        // H5 south corridor
        drawLowPalmTree({-24.f, 0.f, -38.f}, 3.0f, texPalmCrown[0], vp);
        drawLowPalmTree({-18.f, 0.f, -44.f}, 2.6f, texPalmCrown[0], vp);
        // H6 / H7 bottom
        drawLowPalmTree({  2.f, 0.f, -44.f}, 3.0f, texPalmCrown[0], vp);
        drawLowPalmTree({ 12.f, 0.f, -40.f}, 2.8f, texPalmCrown[0], vp);
        // East side / pond area
        drawLowPalmTree({ 20.f, 0.f,   2.f}, 3.2f, texPalmCrown[0], vp);
        drawLowPalmTree({ 22.f, 0.f,  -8.f}, 2.6f, texPalmCrown[0], vp);
        // H9 / H10 upper right approach
        drawLowPalmTree({ 32.f, 0.f,  28.f}, 2.8f, texPalmCrown[0], vp);
        drawLowPalmTree({ 28.f, 0.f,  18.f}, 3.0f, texPalmCrown[0], vp);
        // North central area
        drawLowPalmTree({  0.f, 0.f,  32.f}, 2.7f, texPalmCrown[0], vp);
        drawLowPalmTree({  8.f, 0.f,  38.f}, 3.2f, texPalmCrown[0], vp);

        // Hill terrain — upper-right corner (smoothly blends into flat ground)
        draw(mTerrainHills, mat4(1), vp, 23);

        // Boulders — partially embedded (center at terrainH + r*0.35, sinking ~50% in)
        drawBoulder({38.f, terrainH(38.f,43.f)+3.2f*0.35f, 43.f}, 3.2f, texRock[0], vp);
        drawBoulder({45.f, terrainH(45.f,41.f)+3.8f*0.35f, 41.f}, 3.8f, texRock[1], vp);
        drawBoulder({36.f, terrainH(36.f,48.f)+2.8f*0.35f, 48.f}, 2.8f, texRock[2], vp);
        drawBoulder({47.f, terrainH(47.f,47.f)+2.2f*0.35f, 47.f}, 2.2f, texRock[3], vp);
        drawBoulder({41.f, terrainH(41.f,46.f)+1.9f*0.35f, 46.f}, 1.9f, texRock[4], vp);
        drawBoulder({46.f, terrainH(46.f,44.f)+1.6f*0.35f, 44.f}, 1.6f, texRock[5], vp);
        // Base boulders — nestled at the foot of the hill (use hillH for edge-faded surface height)
        drawBoulder({36.f, hillH(36.f,39.5f)+2.2f*0.35f, 39.5f}, 2.2f, texRock[6], vp);
        drawBoulder({43.f, hillH(43.f,39.5f)+1.8f*0.35f, 39.5f}, 1.8f, texRock[7], vp);

        // Weeds right next to each boulder
        drawBillboard({36.5f,terrainH(36.5f,44.f),   44.f},  1.6f, 2.0f, texWeed[0], vp);
        drawBillboard({39.5f,terrainH(39.5f,42.f),   42.f},  1.4f, 1.7f, texWeed[1], vp);
        drawBillboard({43.5f,terrainH(43.5f,40.5f),  40.5f}, 1.5f, 1.8f, texWeed[2], vp);
        drawBillboard({46.5f,terrainH(46.5f,42.5f),  42.5f}, 1.3f, 1.6f, texWeed[0], vp);
        drawBillboard({34.5f,terrainH(34.5f,47.5f),  47.5f}, 1.4f, 1.7f, texWeed[1], vp);
        drawBillboard({40.f, terrainH(40.f, 45.5f),  45.5f}, 1.2f, 1.5f, texWeed[2], vp);
        drawBillboard({46.5f,terrainH(46.5f,43.5f),  43.5f}, 1.3f, 1.6f, texWeed[0], vp);

        // Palm trees — cylinder trunk with bark texture, crossed billboard crown
        drawPalmTree({35.f, terrainH(35.f,42.f), 42.f}, 6.5f, texBark, texPalmCrown[0], vp);
        drawPalmTree({43.f, terrainH(43.f,45.f), 45.f}, 7.5f, texBark, texPalmCrown[0], vp);
        drawPalmTree({38.f, terrainH(38.f,50.f), 50.f}, 5.5f, texBark, texPalmCrown[0], vp);
        drawPalmTree({47.f, terrainH(47.f,44.f), 44.f}, 5.0f, texBark, texPalmCrown[0], vp);

        // Tropical shrubs
        drawBillboard({44.f, terrainH(44.f,48.f), 48.f}, 2.5f, 3.0f, texShrub[0], vp);
        drawBillboard({34.f, terrainH(34.f,46.f), 46.f}, 2.8f, 3.5f, texShrub[1], vp);

        // Cement walkways between holes and to restaurant
        drawWalkway({-33.f,0.f,29.f}, {-33.f,0.f,25.f}, 1.5f, vp);   // H1 → H2
        drawWalkway({-33.f,0.f,11.f}, {-33.f,0.f, 6.f}, 1.5f, vp);   // H2 → H3
        drawWalkway({-33.f,0.f,-8.f}, {-35.f,0.f,-13.f}, 1.2f, vp);  // H3 → H4
        drawWalkway({-36.f,0.f,-22.f},{-33.f,0.f,-27.f}, 1.2f, vp);  // H4 → H5
        drawWalkway({-33.f,0.f,-36.f},{-28.f,0.f,-41.f}, 1.5f, vp);  // H5 → H6
        drawWalkway({-20.f,0.f,-47.f},{-12.f,0.f,-47.f}, 1.2f, vp);  // H6 → H7
        drawWalkway({ 7.f, 0.f,-47.f},{ 14.f,0.f,-47.f}, 1.2f, vp);  // H7 → H8
        drawWalkway({ 21.f, 0.f,-46.5f},{25.f,0.f,-45.5f}, 1.2f, vp); // H8 → H9
        drawWalkway({ 31.f, 0.f,-45.5f},{34.f,0.f,-39.f},  1.2f, vp); // H9 → H10
        drawWalkway({  5.0f,0.f, 0.f},{ 10.f,0.f,  0.f}, 1.5f, vp);  // restaurant east
        drawWalkway({  0.f, 0.f,-4.0f},{  0.f,0.f,-7.0f},1.5f, vp);  // restaurant south
        drawWalkway(gHoles[1]->getTeePos() + vec3( 5.f,0.f, 2.f), {-3.f,0.f,5.f}, 2.0f, vp); // H1 → restaurant (starts outside H1 south wall)
        drawWalkway(gHoles[6]->getTeePos() + vec3( 7.f,0.f, 3.f), {-3.f,0.f,5.f}, 2.0f, vp); // H6 → restaurant

        // Golf ball
        if(ball.active){
            mat4 m = glm::translate(mat4(1),ball.pos);
            m = glm::scale(m,{BALL_R*2,BALL_R*2,BALL_R*2});
            draw(mSphere,m,vp,15);
        }

        // Drone body — only visible in external-cam mode
        if(!droneView)
            drawDrone(vp);

        // Aim indicator
        if(aimMode && ball.active &&
           glfwGetMouseButton(gWin,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS){
            vec3 diff = {aimTarget.x - ball.pos.x, 0, aimTarget.z - ball.pos.z};
            float len = glm::length(diff);
            if(len > 0.05f){
                vec3 dir = diff / len;
                float ang = atan2f(dir.x, dir.z);
                float lineY = ball.pos.y + 0.12f;
                vec3 mid  = {ball.pos.x + dir.x*(len*0.5f),
                             lineY,
                             ball.pos.z + dir.z*(len*0.5f)};
                mat4 m = glm::translate(mat4(1), mid);
                m = glm::rotate(m, ang, {0,1,0});
                m = glm::scale(m, {0.07f, 0.07f, len});
                draw(mBox, m, vp, 17);
            }
        }

        glfwSwapBuffers(gWin);
        glfwPollEvents();
    }

    for(int i = 1; i <= 12; i++) delete gHoles[i];
    freeMesh(mQuad); freeMesh(mBox); freeMesh(mSphere);
    freeMesh(mCylinder); freeMesh(mTorus); freeMesh(mTrap); freeMesh(mCircle);
    freeMesh(mVQuad); freeMesh(mLeafVQuad); freeMesh(mRockyBoulder); freeMesh(mTerrainHills);
    freeMesh(mRiver); freeMesh(mRiverBank); freeMesh(mTaperedCyl); freeMesh(mThickCyl);
    freeMesh(mRestaurantPlateau); freeMesh(mTerrainWest);
    glDeleteTextures(8, texRock);
    glDeleteTextures(1, &texBark);
    glDeleteTextures(3, texPalmCrown);
    glDeleteTextures(3, texWeed);
    glDeleteTextures(2, texShrub);
    glDeleteTextures(1, &texConcrete);
    glDeleteTextures(1, &texAgave);
    glDeleteTextures(1, &texFlax);
    glDeleteTextures(1, &texLowPalmCrown);
    glDeleteBuffers(1,&mSkybox.vbo);
    glDeleteVertexArrays(1,&mSkybox.vao);
    glDeleteTextures(1, &skyTex);
    glDeleteProgram(gProg); glDeleteProgram(skyProg);
    glfwDestroyWindow(gWin);
    glfwTerminate();
    return 0;
}