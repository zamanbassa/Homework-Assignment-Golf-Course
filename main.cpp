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
#include "Hole13.h"
#include "Hole14.h"
#include "Hole15.h"
#include "Hole16.h"
#include "Hole17.h"
#include "Hole18.h"

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
static GLuint    gProg, skyProg, skyTex;
static Mesh      mQuad, mBox, mSphere, mCylinder, mTorus, mTrap, mSkybox, mCircle;
static Mesh      mTerrainHills;
static Mesh      mVQuad;        // vertical quad for billboards
static Mesh      mRockyBoulder; // low-poly displaced sphere
static Mesh      mRiver, mRiverBank;
static Mesh      mTaperedCyl;

// ─── Texture handles ──────────────────────────────────────────────────────────
static GLuint texRock[8];       // rocks/rock1-8.bmp
static GLuint texBark;          // rocks/wood4.bmp  (palm trunk bark)
static GLuint texPalmCrown[3];  // tropical/t_palm_shrubN/palmsN.png  (leaf crown)
static GLuint texWeed[3];       // tropical/t_trop_weedN/tropwN.png
static GLuint texShrub[2];      // tropical/t_trop_shrubN/tropsN.png
static GLuint texConcrete;      // rocks/floor1.bmp (concrete slabs)
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
    // Restaurant area (corner lamps — outside walls of enlarged 10×8 building)
    { -6.f,  0.f,   5.f},
    {  6.f,  0.f,   5.f},
    { -6.f,  0.f,  -5.f},
    {  6.f,  0.f,  -5.f},
    // Pond lamp (right bank)
    { 24.f,  0.f,  15.f},
    // Hole 17 — Final challenge (L-shape) area lamps
    { 27.5f, 0.f,  43.5f},  // east of A arm (near tee)
    { 27.5f, 0.f,  40.0f},  // east of corner
    { 20.0f, 0.f,  38.0f},  // mid B arm north
    { 15.0f, 0.f,  41.5f},  // west of cup
    // Hole 18 — Lighthouse plaza lamps (around 8×8 plaza at x=12-20, z=45-53)
    { 10.5f, 0.f,  44.0f},  // outside NW
    { 21.5f, 0.f,  44.0f},  // outside NE
    { 10.5f, 0.f,  54.0f},  // outside SW
    { 21.5f, 0.f,  54.0f},  // outside SE
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

static int gCurrentHole = 1;
static Hole* gHoles[19] = {};

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

static void drawRestaurant(const mat4& vp){
    float cx = 0.f, cz = 0.f;
    float bw = 10.f, bd = 8.f, bh = 3.5f;
    { mat4 m = glm::translate(mat4(1),{cx,0.01f,cz}); m = glm::scale(m,{bw,1,bd}); draw(mQuad,m,vp,9); }
    {mat4 m=glm::translate(mat4(1),{cx,bh*.5f,cz-bd*.5f}); m=glm::scale(m,{bw,.0f+bh,.28f}); draw(mBox,m,vp,8);}
    {mat4 m=glm::translate(mat4(1),{cx,bh*.5f,cz+bd*.5f}); m=glm::scale(m,{bw,bh,.28f}); draw(mBox,m,vp,8);}
    {mat4 m=glm::translate(mat4(1),{cx-bw*.5f,bh*.5f,cz}); m=glm::scale(m,{.28f,bh,bd}); draw(mBox,m,vp,8);}
    {mat4 m=glm::translate(mat4(1),{cx+bw*.5f,bh*.5f,cz}); m=glm::scale(m,{.28f,bh,bd}); draw(mBox,m,vp,8);}
    { mat4 m=glm::translate(mat4(1),{cx,bh+.2f,cz}); m=glm::scale(m,{bw+.6f,.4f,bd+.6f}); draw(mBox,m,vp,20); }
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
    mRockyBoulder = makeRockyBoulder();
    mTerrainHills = makeHillTerrain(33.f, 38.f, 16.f, 14.f, 32, 28);
    mTaperedCyl   = makeTaperedCylinder(0.18f, 0.10f, 18);

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
        "textures/tropical/t_palm_shrub4/palms4.png",
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
    gHoles[13] = new Hole13(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[14] = new Hole14(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[15] = new Hole15(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[16] = new Hole16(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[17] = new Hole17(&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[18] = new Hole18(&mQuad, &mBox, &mCylinder, &mTorus);

    double prev = glfwGetTime();

    while(!glfwWindowShouldClose(gWin)){
        double now = glfwGetTime();
        float  dt  = (float)(now-prev);
        prev = now;

        // Time of day
        timeOfDay = fmodf(timeOfDay + todSpeed*dt, 1.f);

        // Propeller spin
        propAngle = fmodf(propAngle + dt * 18.f, 2*PI);

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
            if(gCurrentHole < 18){
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

        for(int i = 1; i <= 18; i++)
            if(gHoles[i]) gHoles[i]->render(vp);

        drawRestaurant(vp);

        // Concrete slabs surrounding restaurant (bw=10, bd=8 → walls at x=±5, z=±4)
        // Front slab: z=4..8 (depth 4) to fully contain front seating
        { mat4 m=glm::translate(mat4(1),{0.f,0.005f, 6.f}); m=glm::scale(m,{16.f,1.f,4.f}); drawWithTex(mQuad,m,vp,8,texConcrete); }
        // Back slab: z=-4..-7
        { mat4 m=glm::translate(mat4(1),{0.f,0.005f,-5.5f}); m=glm::scale(m,{16.f,1.f,3.f}); drawWithTex(mQuad,m,vp,8,texConcrete); }
        // Left slab: x=-5..-8
        { mat4 m=glm::translate(mat4(1),{-6.5f,0.005f,0.f}); m=glm::scale(m,{3.f,1.f, 8.f}); drawWithTex(mQuad,m,vp,8,texConcrete); }
        // Right slab: x=5..9 (width 4 to contain right seating)
        { mat4 m=glm::translate(mat4(1),{ 7.f,  0.005f,0.f}); m=glm::scale(m,{4.f,1.f, 8.f}); drawWithTex(mQuad,m,vp,8,texConcrete); }

        // ── Seating area ────────────────────────────────────────────────────────
        // Front patio: 2 tables at z=6.0 (centred on front slab z=4..8)
        // Chair rot: left of table → +PI/2 (face +X), right → -PI/2 (face -X),
        //            front (z-) → 0 (face +Z), back (z+) → PI (face -Z)
        drawTable({-3.5f, 0.f, 6.0f}, vp);
        drawChair({-4.5f, 0.f, 6.0f},  1.57f, vp);   // left of table → face +X
        drawChair({-2.5f, 0.f, 6.0f}, -1.57f, vp);   // right of table → face -X
        drawChair({-3.5f, 0.f, 7.0f},  3.14f, vp);   // behind table → face -Z
        drawChair({-3.5f, 0.f, 5.0f},  0.0f,  vp);   // in front → face +Z

        drawTable({ 3.5f, 0.f, 6.0f}, vp);
        drawChair({ 2.5f, 0.f, 6.0f},  1.57f, vp);
        drawChair({ 4.5f, 0.f, 6.0f}, -1.57f, vp);
        drawChair({ 3.5f, 0.f, 7.0f},  3.14f, vp);
        drawChair({ 3.5f, 0.f, 5.0f},  0.0f,  vp);

        // Right-side patio: 1 table at (7.5, 0, 1.5) — on right slab x=5..9
        drawTable({7.5f, 0.f, 1.5f}, vp);
        drawChair({6.5f, 0.f, 1.5f},  1.57f, vp);
        drawChair({8.5f, 0.f, 1.5f}, -1.57f, vp);
        drawChair({7.5f, 0.f, 2.5f},  3.14f, vp);
        drawChair({7.5f, 0.f, 0.5f},  0.0f,  vp);

        // ── Corner gardens: dirt patch + small boulder + plant(s) ──────────────
        // Front-left corner of front slab
        { mat4 m=glm::translate(mat4(1),{-6.5f,0.02f, 7.5f}); m=glm::scale(m,{1.6f,1.f,1.6f}); draw(mQuad,m,vp,23); }
        drawBoulder({-6.5f, 0.4f*0.35f, 7.5f}, 0.40f, texRock[2], vp);
        drawBillboard({-6.8f, 0.f, 7.2f}, 0.65f, 1.0f, texWeed[0], vp);
        drawBillboard({-6.2f, 0.f, 7.8f}, 0.55f, 0.85f, texWeed[1], vp);

        // Front-right corner of front slab
        { mat4 m=glm::translate(mat4(1),{ 6.5f,0.02f, 7.5f}); m=glm::scale(m,{1.6f,1.f,1.6f}); draw(mQuad,m,vp,23); }
        drawBoulder({ 6.5f, 0.35f*0.35f, 7.5f}, 0.35f, texRock[3], vp);
        drawBillboard({ 6.2f, 0.f, 7.2f}, 0.60f, 0.90f, texWeed[0], vp);

        // Right-slab front corner
        { mat4 m=glm::translate(mat4(1),{ 8.5f,0.02f, 3.5f}); m=glm::scale(m,{1.6f,1.f,1.6f}); draw(mQuad,m,vp,23); }
        drawBoulder({ 8.5f, 0.45f*0.35f, 3.5f}, 0.45f, texRock[4], vp);
        drawBillboard({ 8.8f, 0.f, 3.2f}, 0.60f, 0.95f, texWeed[1], vp);
        drawBillboard({ 8.2f, 0.f, 3.8f}, 0.50f, 0.80f, texWeed[0], vp);

        // Right-slab back corner
        { mat4 m=glm::translate(mat4(1),{ 8.5f,0.02f,-3.5f}); m=glm::scale(m,{1.6f,1.f,1.6f}); draw(mQuad,m,vp,23); }
        drawBoulder({ 8.5f, 0.38f*0.35f,-3.5f}, 0.38f, texRock[5], vp);
        drawBillboard({ 8.8f, 0.f,-3.8f}, 0.55f, 0.85f, texWeed[1], vp);

        // Lamp posts
        for(int i = 0; i < LAMP_COUNT; i++)
            drawLampPost(LAMP_POSITIONS[i], vp);

        // Pond island — drawn before water so depth test blocks water over the island
        { mat4 m = glm::translate(mat4(1), {14.f, -0.3f, 16.f});
          m = glm::scale(m, {3.5f, 0.85f, 3.5f});
          draw(mSphere, m, vp, 4); }  // sand/gravel dome; island top ≈ y=0.55

        // River + pond (drawn after island so water fills around the dome)
        draw(mRiverBank, mat4(1), vp, 23);  // dirt/soil banks
        draw(mRiver,     mat4(1), vp,  5);  // animated water

        // Island vegetation — boulders, palm and plants grouped on the small hill
        drawBoulder({15.8f, 0.45f, 15.2f}, 0.30f, texRock[1], vp);
        drawBoulder({12.5f, 0.38f, 17.2f}, 0.25f, texRock[3], vp);
        drawPalmTree({14.f,  0.52f, 16.5f}, 4.5f, texBark, texPalmCrown[0], vp);
        drawBillboard({15.3f, 0.35f, 15.0f}, 0.45f, 0.75f, texWeed[0], vp);
        drawBillboard({12.8f, 0.28f, 17.5f}, 0.40f, 0.65f, texWeed[2], vp);
        drawBillboard({14.8f, 0.30f, 14.3f}, 0.50f, 0.70f, texWeed[1], vp);
        drawBillboard({13.5f, 0.38f, 17.9f}, 1.0f,  1.3f,  texShrub[0], vp);

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

        // Hole 14 (smooth S-curve, was H15 before renumber) — smaller decorative palms
        {
            const auto& tp = Hole14::treePositions();
            for (size_t i = 0; i < tp.size(); i++) {
                float h = 2.6f + 0.4f * (float)((i * 13) % 5) / 4.f;  // slight variation
                drawPalmTree(tp[i], h, texBark, texPalmCrown[i % 3], vp);
            }
        }

        // Hole 18 (lighthouse plaza) — corner decorative palms
        {
            const auto& tp = Hole18::treePositions();
            for (size_t i = 0; i < tp.size(); i++) {
                drawPalmTree(tp[i], 3.2f, texBark, texPalmCrown[i % 3], vp);
            }
        }

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

    for(int i = 1; i <= 18; i++) delete gHoles[i];
    freeMesh(mQuad); freeMesh(mBox); freeMesh(mSphere);
    freeMesh(mCylinder); freeMesh(mTorus); freeMesh(mTrap); freeMesh(mCircle);
    freeMesh(mVQuad); freeMesh(mRockyBoulder); freeMesh(mTerrainHills);
    freeMesh(mRiver); freeMesh(mRiverBank); freeMesh(mTaperedCyl);
    glDeleteTextures(8, texRock);
    glDeleteTextures(1, &texBark);
    glDeleteTextures(3, texPalmCrown);
    glDeleteTextures(3, texWeed);
    glDeleteTextures(2, texShrub);
    glDeleteTextures(1, &texConcrete);
    glDeleteBuffers(1,&mSkybox.vbo);
    glDeleteVertexArrays(1,&mSkybox.vao);
    glDeleteTextures(1, &skyTex);
    glDeleteProgram(gProg); glDeleteProgram(skyProg);
    glfwDestroyWindow(gWin);
    glfwTerminate();
    return 0;
}