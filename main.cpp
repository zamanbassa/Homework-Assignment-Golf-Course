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
// Vertex / Mesh types come from Mesh.h (included via Hole*.h)

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
void freeMesh(Mesh& m){
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

// Filled arc strip in XZ plane (annular sector floor)
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

// Flat triangle in XZ plane, facing up
Mesh makeTriFloor(vec3 a, vec3 b, vec3 c){
    vector<Vertex> V={
        {a,{0,1,0},{0.0f,0.5f}},
        {b,{0,1,0},{0.0f,1.0f}},
        {c,{0,1,0},{1.0f,0.5f}},
    };
    return upload(V,{0,1,2});
}

// Horizontal quad in XZ plane, UV 0-1
Mesh makeQuad(){
    vector<Vertex> V={
        {{-0.5f,0,-0.5f},{0,1,0},{0,0}},
        {{ 0.5f,0,-0.5f},{0,1,0},{1,0}},
        {{ 0.5f,0, 0.5f},{0,1,0},{1,1}},
        {{-0.5f,0, 0.5f},{0,1,0},{0,1}},
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

// Sphere (for ball and hill dome)
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

// Cylinder (flag pole, pillars)
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

// Torus (cup rim)
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
Mesh makeTrapezoid(){
    vector<Vertex> V={
        {{-40,0,-55},{0,1,0},{0.03f,0}},
        {{ 40,0,-55},{0,1,0},{0.97f,0}},
        {{ 50,0, 55},{0,1,0},{1.00f,1}},
        {{-50,0, 55},{0,1,0},{0.00f,1}},
    };
    return upload(V,{0,1,2,0,2,3});
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
static GLFWwindow* gWin = nullptr;
static float     timeOfDay = 0.45f;
static float     todSpeed  = 0.002f;

// Hole objects (indices 1-10; index 0 unused)
static Hole* gHoles[11] = {};

// ─── Lamp post positions (world XZ, y=0 base) ─────────────────────────────────
static vec3 LAMP_POSITIONS[] = {
    {-30.5f, 0.f,  31.f},   // H1 cup east
    {-35.5f, 0.f,  31.f},   // H1 cup west
    {-30.5f, 0.f,  42.f},   // H1 tee area
    {-30.5f, 0.f,  24.f},   // between H1/H2
    {-30.5f, 0.f,  13.f},   // H2 cup east
    {-35.5f, 0.f,  13.f},   // H2 cup west
    {-30.5f, 0.f,   4.f},   // between H2/H3
    {-30.5f, 0.f,  -7.f},   // H3 cup area
    {-35.5f, 0.f, -20.f},   // H4 cup area
    {-30.5f, 0.f, -35.f},   // H5 cup area
    {-35.5f, 0.f, -44.f},   // H6 tee/fairway
    {-21.f,  0.f, -49.f},   // H6 cup area
    {  4.f,  0.f, -50.f},   // H7 cup area
    { 10.f,  0.f, -48.f},   // between H7/H8
    { 22.f,  0.f, -49.f},   // H8 cup area
    { -5.f,  0.f,   4.5f},  // restaurant corner
    {  5.f,  0.f,   4.5f},
    { -5.f,  0.f,  -3.5f},
    {  5.f,  0.f,  -3.5f},
};
static const int LAMP_COUNT = (int)(sizeof(LAMP_POSITIONS)/sizeof(LAMP_POSITIONS[0]));

// ─── Drone state ──────────────────────────────────────────────────────────────
struct Drone {
    vec3  pos   = {-33.f, 12.f, 20.f};
    float yaw   = 0.f;
    float pitch = -0.4f;
} drone;

static bool  droneView  = true;
static float spotYawOff = 0.f;
static float spotPitOff = 0.f;
static float propAngle  = 0.f;

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
static const float MAX_AIM =  6.0f;

static int gCurrentHole = 1;

struct Ball {
    vec3  pos    = {H1_CX, BALL_R_CONST, H1_CZ+5.5f};
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
        if(gHoles[gCurrentHole])
            gHoles[gCurrentHole]->wallCollide(pos, vel);
    }

    bool nearCup() const {
        if(gHoles[gCurrentHole]) return gHoles[gCurrentHole]->nearCup(pos);
        return false;
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

// Clamp raw ground point to MAX_AIM distance from ball
static void updateAimTarget(double mx, double my){
    vec3 raw  = mouseToGround(mx, my);
    vec3 diff = {raw.x - ball.pos.x, 0.f, raw.z - ball.pos.z};
    float d   = glm::length(diff);
    if(d > 0.001f && d > MAX_AIM)
        aimTarget = {ball.pos.x + diff.x/d*MAX_AIM, 0.f, ball.pos.z + diff.z/d*MAX_AIM};
    else
        aimTarget = {raw.x, 0.f, raw.z};
}

// ─── Callbacks ───────────────────────────────────────────────────────────────
static void cbScroll(GLFWwindow*, double, double dy){
    cam.fov = glm::clamp(cam.fov - (float)dy*2.5f, 10.f, 95.f);
}

static void cbMouseBtn(GLFWwindow*, int btn, int act, int){
    if(btn == GLFW_MOUSE_BUTTON_RIGHT){
        rmbDown = (act == GLFW_PRESS);
        skipFirst = true;
        if(rmbDown)
            glfwSetInputMode(gWin, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(gWin, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
        drone.yaw   += dx;
        drone.pitch -= dy;
        drone.pitch  = glm::clamp(drone.pitch, -1.4f, 1.4f);
    } else {
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

    case GLFW_KEY_F:
        droneView = !droneView;
        if(droneView){
            cam.pos   = drone.pos;
            cam.yaw   = drone.yaw;
            cam.pitch = drone.pitch;
        } else {
            cam.pos   = drone.pos + vec3(-sinf(drone.yaw)*8.f, 3.f, cosf(drone.yaw)*8.f);
            cam.yaw   = drone.yaw;
            cam.pitch = -0.25f;
        }
        break;

    case GLFW_KEY_G:
        if(ball.active && !ball.moving) aimMode = !aimMode; break;
    case GLFW_KEY_SPACE: {
        if(gHoles[gCurrentHole]){
            vec3 tp = gHoles[gCurrentHole]->getTeePos();
            ball.pos    = tp;
            ball.vel    = {0,0,0};
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
        todSpeed = (todSpeed > 0) ? 0 : 0.002f; break;
    case GLFW_KEY_EQUAL:  cam.fov = glm::max(cam.fov-3.f, 10.f); break;
    case GLFW_KEY_MINUS:  cam.fov = glm::min(cam.fov+3.f, 95.f); break;

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
        glUniform3fv(glGetUniformLocation(gProg,"uLampPos"), LAMP_COUNT, glm::value_ptr(lampWorldPos[0]));
    }
    vec3 lampCol = {1.0f, 0.92f, 0.65f};
    glUniform3fv(glGetUniformLocation(gProg,"uLampColor"), 1, glm::value_ptr(lampCol));

    int spotOn = nightNow ? 1 : 0;
    float sy = drone.yaw + spotYawOff, sp = drone.pitch + spotPitOff - 0.3f;
    vec3 spotDir = glm::normalize(vec3(sinf(sy)*cosf(sp), sinf(sp), -cosf(sy)*cosf(sp)));
    glUniform1i(glGetUniformLocation(gProg,"uSpotOn"),     spotOn);
    glUniform3fv(glGetUniformLocation(gProg,"uSpotPos"), 1, glm::value_ptr(drone.pos));
    glUniform3fv(glGetUniformLocation(gProg,"uSpotDir"), 1, glm::value_ptr(spotDir));
    glUniform1f(glGetUniformLocation(gProg,"uSpotCutoff"), cosf(glm::radians(18.f)));
    glUniform1f(glGetUniformLocation(gProg,"uSpotOuter"),  cosf(glm::radians(26.f)));
}

// ─── Draw the restaurant ─────────────────────────────────────────────────────
static void drawRestaurant(const mat4& vp){
    float cx = 0.f, cz = 0.f;
    float bw = 7.f, bd = 5.5f, bh = 3.2f;

    { mat4 m=glm::translate(mat4(1),{cx,0.01f,cz}); m=glm::scale(m,{bw,1,bd}); draw(mQuad,m,vp,9); }
    {mat4 m=glm::translate(mat4(1),{cx,bh*.5f,cz-bd*.5f}); m=glm::scale(m,{bw,.0f+bh,.28f}); draw(mBox,m,vp,8);}
    {mat4 m=glm::translate(mat4(1),{cx,bh*.5f,cz+bd*.5f}); m=glm::scale(m,{bw,bh,.28f}); draw(mBox,m,vp,8);}
    {mat4 m=glm::translate(mat4(1),{cx-bw*.5f,bh*.5f,cz}); m=glm::scale(m,{.28f,bh,bd}); draw(mBox,m,vp,8);}
    {mat4 m=glm::translate(mat4(1),{cx+bw*.5f,bh*.5f,cz}); m=glm::scale(m,{.28f,bh,bd}); draw(mBox,m,vp,8);}
    { mat4 m=glm::translate(mat4(1),{cx,bh+.2f,cz}); m=glm::scale(m,{bw+.6f,.4f,bd+.6f}); draw(mBox,m,vp,20); }
}

// ─── Drone ────────────────────────────────────────────────────────────────────
static void drawDrone(const mat4& vp){
    vec3 p = drone.pos;
    mat4 baseRot = glm::rotate(mat4(1), drone.yaw, {0,1,0});

    { mat4 m = glm::translate(mat4(1), p) * baseRot; m = glm::scale(m, {1.4f, 0.28f, 0.9f}); draw(mBox, m, vp, 24); }
    { mat4 m = glm::translate(mat4(1), p + vec3(0, 0.22f, 0)); m = glm::scale(m, {0.38f, 0.22f, 0.38f}); draw(mSphere, m, vp, 24); }

    float armAngles[4] = {PI*0.25f, PI*0.75f, PI*1.25f, PI*1.75f};
    for(int i = 0; i < 4; i++){
        float aa = armAngles[i] + drone.yaw;
        float ax = cosf(aa)*0.9f, az = sinf(aa)*0.9f;
        vec3 tip = p + vec3(ax, 0, az);
        { vec3 mid = p + vec3(ax*0.5f, 0, az*0.5f);
          mat4 m = glm::translate(mat4(1), mid);
          m = glm::rotate(m, aa, {0,1,0}); m = glm::scale(m, {0.12f, 0.08f, 0.9f});
          draw(mBox, m, vp, 24); }
        { mat4 m = glm::translate(mat4(1), tip); m = glm::scale(m, {0.12f, 0.12f, 0.12f}); draw(mSphere, m, vp, 24); }
        for(int b = 0; b < 2; b++){
            float ba = propAngle + b*PI + aa;
            vec3 bpos = tip + vec3(cosf(ba)*0.32f, 0.10f, sinf(ba)*0.32f);
            mat4 m = glm::translate(mat4(1), bpos);
            m = glm::rotate(m, ba, {0,1,0}); m = glm::scale(m, {0.60f, 0.04f, 0.14f});
            draw(mBox, m, vp, 25);
        }
    }
}

// ─── Lamp post ────────────────────────────────────────────────────────────────
static void drawLampPost(vec3 pos, const mat4& vp){
    { mat4 m = glm::translate(mat4(1), pos); m = glm::scale(m, {0.10f, 4.0f, 0.10f}); draw(mCylinder, m, vp, 13); }
    { mat4 m = glm::translate(mat4(1), pos + vec3(0, 4.0f, 0)); m = glm::scale(m, {0.8f, 0.08f, 0.08f}); draw(mBox, m, vp, 13); }
    { mat4 m = glm::translate(mat4(1), pos + vec3(0.4f, 4.0f, 0)); m = glm::scale(m, {0.25f, 0.25f, 0.25f}); draw(mSphere, m, vp, 14); }
}

// ─── Skybox ───────────────────────────────────────────────────────────────────
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
    mSkybox   = makeSkyboxMesh();

    // Create hole objects — meshes built in constructors
    gHoles[1]  = new Hole1 (&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[2]  = new Hole2 (&mQuad, &mBox, &mCylinder, &mTorus, &mSphere);
    gHoles[3]  = new Hole3 (&mQuad, &mBox, &mCylinder, &mTorus, &mCircle);
    gHoles[4]  = new Hole4 (&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[5]  = new Hole5 (&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[6]  = new Hole6 (&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[7]  = new Hole7 (&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[8]  = new Hole8 (&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[9]  = new Hole9 (&mQuad, &mBox, &mCylinder, &mTorus);
    gHoles[10] = new Hole10(&mQuad, &mBox, &mCylinder, &mTorus);

    double prev = glfwGetTime();

    while(!glfwWindowShouldClose(gWin)){
        double now = glfwGetTime();
        float  dt  = (float)(now-prev);
        prev = now;

        timeOfDay = fmodf(timeOfDay + todSpeed*dt, 1.f);
        propAngle = fmodf(propAngle + dt * 18.f, 2*PI);

        ball.update(dt);

        // Terrain effects: hill slope / height (delegated to hole object)
        if(ball.active){
            Hole* h = gHoles[gCurrentHole];
            if(h){
                ball.pos.y = h->groundY(ball.pos);
                vec3 tf = h->terrainForce(ball.pos);
                if(glm::length(tf) > 0.001f){
                    ball.vel += tf * dt;
                    if(glm::length(ball.vel) > 0.01f) ball.moving = true;
                }
            } else {
                ball.pos.y = BALL_R_CONST;
            }
        }

        // Hole completion → advance to next hole
        if(ball.active && !ball.inHole && ball.nearCup()){
            ball.inHole = true;
            ball.moving = false;
            printf("Hole %d: %d stroke%s\n",
                   gCurrentHole, ball.strokes, ball.strokes==1?"":"s");
            if(gCurrentHole < 10){
                gCurrentHole++;
                if(gHoles[gCurrentHole]){
                    vec3 tp = gHoles[gCurrentHole]->getTeePos();
                    ball.pos = tp;
                }
                ball.vel    = {0,0,0};
                ball.active = true; ball.moving = false;
                ball.inHole = false; ball.strokes = 0;
                aimMode = false;
            } else {
                ball.active = false;
                printf("Course complete!\n");
            }
        }

        // Camera / drone movement
        float spd = cam.spd * dt;
        if(droneView){
            vec3 fw = {sinf(drone.yaw)*cosf(drone.pitch), sinf(drone.pitch), -cosf(drone.yaw)*cosf(drone.pitch)};
            vec3 rt = glm::normalize(glm::cross(fw,{0,1,0}));
            if(glfwGetKey(gWin,GLFW_KEY_W)==GLFW_PRESS) drone.pos += fw*spd;
            if(glfwGetKey(gWin,GLFW_KEY_S)==GLFW_PRESS) drone.pos -= fw*spd;
            if(glfwGetKey(gWin,GLFW_KEY_A)==GLFW_PRESS) drone.pos -= rt*spd;
            if(glfwGetKey(gWin,GLFW_KEY_D)==GLFW_PRESS) drone.pos += rt*spd;
            if(glfwGetKey(gWin,GLFW_KEY_Q)==GLFW_PRESS) drone.pos.y -= spd;
            if(glfwGetKey(gWin,GLFW_KEY_E)==GLFW_PRESS) drone.pos.y += spd;
            cam.pos   = drone.pos;
            cam.yaw   = drone.yaw;
            cam.pitch = drone.pitch;
        } else {
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

        // Skybox
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

        // Scene
        glUseProgram(gProg);
        setUniforms((float)now, vp);

        // Property grass
        draw(mTrap, mat4(1), vp, 2);

        // Holes 1-10
        for(int i=1; i<=10; i++){
            if(gHoles[i]) gHoles[i]->render(vp);
        }

        // Restaurant
        drawRestaurant(vp);

        // Lamp posts
        for(int i = 0; i < LAMP_COUNT; i++)
            drawLampPost(LAMP_POSITIONS[i], vp);

        // Drone (visible only in external-cam mode)
        if(!droneView) drawDrone(vp);

        // Golf ball
        if(ball.active){
            mat4 m = glm::translate(mat4(1),ball.pos);
            m = glm::scale(m,{BALL_R_CONST*2, BALL_R_CONST*2, BALL_R_CONST*2});
            draw(mSphere,m,vp,15);
        }

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

    // Cleanup hole objects (destroys their owned meshes)
    for(int i=1; i<=10; i++){ delete gHoles[i]; gHoles[i] = nullptr; }

    freeMesh(mQuad); freeMesh(mBox); freeMesh(mSphere);
    freeMesh(mCylinder); freeMesh(mTorus); freeMesh(mTrap); freeMesh(mCircle);
    glDeleteBuffers(1,&mSkybox.vbo);
    glDeleteVertexArrays(1,&mSkybox.vao);
    glDeleteTextures(1, &skyTex);
    glDeleteProgram(gProg); glDeleteProgram(skyProg);
    glfwDestroyWindow(gWin);
    glfwTerminate();
    return 0;
}
