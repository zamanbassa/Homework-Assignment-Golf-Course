/* ============================================================
   COS344 HA — 18-Hole Miniature Golf Course
   Adventure Golf Fourways-inspired tropical layout

   See controls.txt for all keybindings.
   ============================================================ */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

using namespace std;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

// ─── Constants ───────────────────────────────────────────────────────────────
static const float PI  = 3.14159265358979f;
static const int   WIN_W = 1280, WIN_H = 800;

// ─── Geometry helpers ────────────────────────────────────────────────────────
struct Vertex { vec3 pos, norm; vec2 uv; };
struct Mesh   { GLuint vao, vbo, ebo; int count; };




static Mesh uploadMesh(const vector<Vertex>& verts, const vector<unsigned>& idx){
    Mesh m;
    m.count = (int)idx.size();
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*sizeof(unsigned), idx.data(), GL_STATIC_DRAW);
    // pos
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,pos));
    glEnableVertexAttribArray(0);
    // norm
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,norm));
    glEnableVertexAttribArray(1);
    // uv
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,uv));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    return m;
}

static void freeMesh(Mesh& m){
    glDeleteBuffers(1,&m.vbo);
    glDeleteBuffers(1,&m.ebo);
    glDeleteVertexArrays(1,&m.vao);
}

// Unit quad in XZ plane (y=0), facing up, UV 0-1
static Mesh makeQuad(){
    vector<Vertex> v = {
        {{-0.5f,0,-0.5f},{0,1,0},{0,0}},
        {{ 0.5f,0,-0.5f},{0,1,0},{1,0}},
        {{ 0.5f,0, 0.5f},{0,1,0},{1,1}},
        {{-0.5f,0, 0.5f},{0,1,0},{0,1}},
    };
    vector<unsigned> i = {0,1,2,0,2,3};
    return uploadMesh(v,i);
}

// Box [-0.5,0.5]^3
static Mesh makeBox(){
    // 6 faces
    vector<Vertex> verts;
    vector<unsigned> idx;
    // face normals and offsets
    struct Face { vec3 n; vec3 u; vec3 v; vec3 orig; };
    Face faces[6] = {
        {{0,0,1},{1,0,0},{0,1,0},{-0.5f,-0.5f,0.5f}},   // front
        {{0,0,-1},{-1,0,0},{0,1,0},{0.5f,-0.5f,-0.5f}},  // back
        {{1,0,0},{0,0,-1},{0,1,0},{0.5f,-0.5f,0.5f}},    // right
        {{-1,0,0},{0,0,1},{0,1,0},{-0.5f,-0.5f,-0.5f}},  // left
        {{0,1,0},{1,0,0},{0,0,-1},{-0.5f,0.5f,0.5f}},    // top (swap v direction)
        {{0,-1,0},{1,0,0},{0,0,1},{-0.5f,-0.5f,-0.5f}},  // bottom
    };
    for (int f=0;f<6;f++){
        unsigned base = (unsigned)verts.size();
        for (int vi=0;vi<4;vi++){
            float uu = (vi==1||vi==2)?1.f:0.f;
            float vv = (vi==2||vi==3)?1.f:0.f;
            vec3 p = faces[f].orig + faces[f].u*uu + faces[f].v*vv;
            verts.push_back({p, faces[f].n, {uu,vv}});
        }
        idx.insert(idx.end(),{base,base+1,base+2,base,base+2,base+3});
    }
    return uploadMesh(verts,idx);
}

// Sphere, stacks x slices
static Mesh makeSphere(int st=16, int sl=24){
    vector<Vertex> verts;
    vector<unsigned> idx;
    for (int i=0;i<=st;i++){
        float phi = PI * i / st;
        for (int j=0;j<=sl;j++){
            float th = 2*PI*j/sl;
            vec3 n = {sinf(phi)*cosf(th), cosf(phi), sinf(phi)*sinf(th)};
            verts.push_back({n, n, {(float)j/sl,(float)i/st}});
        }
    }
    for (int i=0;i<st;i++)
        for (int j=0;j<sl;j++){
            unsigned a=i*(sl+1)+j, b=a+1, c=(i+1)*(sl+1)+j, d=c+1;
            idx.insert(idx.end(),{a,c,b,b,c,d});
        }
    return uploadMesh(verts,idx);
}

// Cylinder (caps included), radius r, height h, slices
static Mesh makeCylinder(float r=0.5f, float h=1.0f, int sl=24){
    vector<Vertex> verts;
    vector<unsigned> idx;
    // side
    for (int i=0;i<=sl;i++){
        float th = 2*PI*i/sl;
        vec3 n = {cosf(th),0,sinf(th)};
        vec3 p0 = {r*cosf(th), 0,        r*sinf(th)};
        vec3 p1 = {r*cosf(th), h,        r*sinf(th)};
        float u = (float)i/sl;
        verts.push_back({p0,n,{u,0}});
        verts.push_back({p1,n,{u,1}});
    }
    for (int i=0;i<sl;i++){
        unsigned a=2*i,b=2*i+1,c=2*(i+1),d=2*(i+1)+1;
        idx.insert(idx.end(),{a,b,c,b,d,c});
    }
    // top cap
    unsigned topCenter = (unsigned)verts.size();
    verts.push_back({{0,h,0},{0,1,0},{0.5f,0.5f}});
    for (int i=0;i<=sl;i++){
        float th=2*PI*i/sl;
        verts.push_back({{r*cosf(th),h,r*sinf(th)},{0,1,0},{0.5f+0.5f*cosf(th),0.5f+0.5f*sinf(th)}});
    }
    for (int i=0;i<sl;i++){
        idx.insert(idx.end(),{topCenter, topCenter+1+i, topCenter+2+i});
    }
    // bottom cap
    unsigned botCenter = (unsigned)verts.size();
    verts.push_back({{0,0,0},{0,-1,0},{0.5f,0.5f}});
    for (int i=0;i<=sl;i++){
        float th=2*PI*i/sl;
        verts.push_back({{r*cosf(th),0,r*sinf(th)},{0,-1,0},{0.5f+0.5f*cosf(th),0.5f+0.5f*sinf(th)}});
    }
    for (int i=0;i<sl;i++){
        idx.insert(idx.end(),{botCenter, botCenter+2+i, botCenter+1+i});
    }
    return uploadMesh(verts,idx);
}

// Cone (base at y=0, tip at y=h)
static Mesh makeCone(float r=0.5f, float h=1.0f, int sl=24){
    vector<Vertex> verts;
    vector<unsigned> idx;
    float slant = sqrtf(r*r+h*h);
    for (int i=0;i<=sl;i++){
        float th=2*PI*i/sl;
        // side normal tilted inward by slope
        vec3 n = {cosf(th)*h/slant, r/slant, sinf(th)*h/slant};
        verts.push_back({{r*cosf(th),0,r*sinf(th)},n,{(float)i/sl,0}});
        verts.push_back({{0,h,0},n,{(float)i/sl,1}});
    }
    for (int i=0;i<sl;i++){
        unsigned a=2*i,b=2*i+1,c=2*(i+1);
        idx.insert(idx.end(),{a,b,c});
    }
    // base cap
    unsigned ctr=(unsigned)verts.size();
    verts.push_back({{0,0,0},{0,-1,0},{0.5f,0.5f}});
    for (int i=0;i<=sl;i++){
        float th=2*PI*i/sl;
        verts.push_back({{r*cosf(th),0,r*sinf(th)},{0,-1,0},{0.5f+0.5f*cosf(th),0.5f+0.5f*sinf(th)}});
    }
    for (int i=0;i<sl;i++) idx.insert(idx.end(),{ctr,ctr+2+i,ctr+1+i});
    return uploadMesh(verts,idx);
}

// Torus (for hole rim/cup)
static Mesh makeTorus(float R=1.0f, float r=0.05f, int sl=32, int st=12){
    vector<Vertex> verts;
    vector<unsigned> idx;
    for (int i=0;i<=sl;i++){
        float th=2*PI*i/sl;
        for (int j=0;j<=st;j++){
            float ph=2*PI*j/st;
            vec3 n={cosf(th)*cosf(ph), sinf(ph), sinf(th)*cosf(ph)};
            vec3 p={(R+r*cosf(ph))*cosf(th), r*sinf(ph), (R+r*cosf(ph))*sinf(th)};
            verts.push_back({p,n,{(float)i/sl,(float)j/st}});
        }
    }
    for (int i=0;i<sl;i++)
        for (int j=0;j<st;j++){
            unsigned a=i*(st+1)+j,b=a+1,c=(i+1)*(st+1)+j,d=c+1;
            idx.insert(idx.end(),{a,c,b,b,c,d});
        }
    return uploadMesh(verts,idx);
}

// ─── Draw helper ─────────────────────────────────────────────────────────────
static void draw(GLuint prog, const Mesh& m, const mat4& model, const mat4& vp, int surf){
    mat4 mvp = vp * model;
    glUniformMatrix4fv(glGetUniformLocation(prog,"uMVP"),  1,GL_FALSE,glm::value_ptr(mvp));
    glUniformMatrix4fv(glGetUniformLocation(prog,"uModel"),1,GL_FALSE,glm::value_ptr(model));
    glUniform1i(glGetUniformLocation(prog,"uSurface"), surf);
    glBindVertexArray(m.vao);
    glDrawElements(GL_TRIANGLES, m.count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// ─── Hole layout data ─────────────────────────────────────────────────────────
struct HoleInfo {
    vec3 tee;     // tee box center
    vec3 cup;     // cup center
    float len;    // fairway length (visual)
    float wid;    // fairway width
    float rot;    // rotation around Y (radians)
    int   par;
    const char* name;
};

// 18 holes arranged in a horseshoe with restaurant in center
// Clockwise flow: holes 1-9 on the outer ring, 10-18 looping back
// All y=0 (flat terrain with raised borders)
static HoleInfo HOLES[18] = {
    // --- Starting straight holes, south side ---
    { {-24,0,-18}, {-24,0,-12}, 6,  3, 0.0f,         3, "Palm Alley"        },
    { {-18,0,-18}, {-12,0,-18}, 6,  3, PI*0.5f,       3, "Coconut Curve"     },
    { {-18,0,-12}, {-12,0,-6},  7,  3, PI*0.25f,      4, "Jungle Bend"       },
    { { -6,0, -6}, {  0,0, -6}, 6,  3, PI*0.5f,       3, "Waterfall"         },
    { {  0,0,-12}, {  6,0,-12}, 6,  3, PI*0.5f,       3, "Sandy Shore"       },
    { {  6,0,-18}, { 12,0,-18}, 6,  3, PI*0.5f,       3, "Rock Garden"       },
    // --- Windmill hole ---
    { { 18,0,-18}, { 18,0,-10}, 8,  3.5f, 0.0f,       4, "Windmill"          },
    { { 24,0,-12}, { 24,0, -6}, 6,  3, 0.0f,          3, "Tropical Oasis"    },
    { { 24,0,  0}, { 18,0,  0}, 6,  3, PI*0.5f,       3, "Lagoon Loop"       },
    // --- Restaurant area turn ---
    { { 18,0,  8}, { 12,0,  8}, 6,  3, PI*0.5f,       3, "Restaurant View"   },
    { {  6,0, 12}, {  0,0, 12}, 6,  3, PI*0.5f,       3, "Bamboo Bridge"     },
    { { -6,0,  8}, {-12,0,  8}, 6,  3, PI*0.5f,       3, "Parrot Perch"      },
    { {-18,0, 10}, {-18,0, 16}, 6,  3, 0.0f,          3, "Flamingo Pond"     },
    { {-24,0, 16}, {-18,0, 22}, 7,  3, PI*0.25f,      4, "Toucan Trail"      },
    { {-12,0, 22}, { -6,0, 22}, 6,  3, PI*0.5f,       3, "Monkey Bridge"     },
    { {  0,0, 18}, {  6,0, 18}, 6,  3, PI*0.5f,       3, "Sunset Cove"       },
    { { 12,0, 22}, { 18,0, 22}, 6,  3, PI*0.5f,       3, "Volcano Vista"     },
    { { 24,0, 16}, { 24,0, 22}, 6,  3, 0.0f,          3, "Island Green"      },
};

// ─── Lamp post positions ──────────────────────────────────────────────────────
static vec3 LAMP_POSITIONS[] = {
    // Perimeter lamps
    {-28,0,-22},{-14,0,-22},{  0,0,-22},{ 14,0,-22},{ 28,0,-22},
    { 28,0, -8},{ 28,0,  6},{ 28,0, 20},
    { 14,0, 26},{  0,0, 26},{-14,0, 26},{-28,0, 26},
    {-28,0, 12},{-28,0, -2},
    // Restaurant area
    { -6,0,  0},{  6,0,  0},{  0,0,  6},{  0,0, -6},
    // Along paths
    {-18,0,  0},{  18,0,-6},
};
static const int LAMP_COUNT = sizeof(LAMP_POSITIONS)/sizeof(LAMP_POSITIONS[0]);

// ─── Camera ──────────────────────────────────────────────────────────────────
struct Camera {
    vec3  pos   = {0,18,28};
    float yaw   = 0.0f;   // radians
    float pitch = -0.55f; // radians, looking slightly down
    float speed = 15.0f;
    bool  ortho = false;
    double lastX=0,lastY=0;
    bool   firstMouse=true;

    mat4 view() const {
        vec3 fwd = {sinf(yaw)*cosf(pitch), sinf(pitch), -cosf(yaw)*cosf(pitch)};
        return glm::lookAt(pos, pos+fwd, vec3(0,1,0));
    }
    mat4 proj(float asp) const {
        if (ortho){
            float h = 30.0f; float w = h*asp;
            return glm::ortho(-w,w,-h,h, 0.1f, 500.f);
        }
        return glm::perspective(glm::radians(60.0f), asp, 0.1f, 500.f);
    }
};

// ─── Ball physics ─────────────────────────────────────────────────────────────
struct Ball {
    vec3  pos    = {0,0.2f,0};
    vec3  vel    = {0,0,0};
    bool  active = false;   // spawned?
    bool  moving = false;
    bool  inHole = false;
    float radius = 0.2f;
    int   holeIdx = 0;      // which hole is being played
    int   strokes = 0;

    // aim drag
    bool  aiming    = false;
    vec2  dragStart = {0,0};
    vec2  dragCur   = {0,0};
    float power     = 0.0f;
    vec3  aimDir    = {0,0,-1};

    void update(float dt){
        if (!active || !moving) return;
        const float friction = 0.92f;
        pos += vel * dt;
        vel *= powf(friction, dt * 60.0f);
        if (glm::length(vel) < 0.01f){
            vel = vec3(0);
            moving = false;
        }
        // gravity keeps ball at y=radius
        pos.y = radius;
    }
    bool nearCup(const vec3& cup) const {
        float d = glm::length(vec3(pos.x-cup.x,0,pos.z-cup.z));
        return d < 0.35f;
    }
};

// ─── Windmill ────────────────────────────────────────────────────────────────
struct Windmill {
    vec3  pos = {18,0,-14};
    float rot = 0.0f; // blade rotation
};

// ─── Game state ───────────────────────────────────────────────────────────────
static Camera   cam;
static Ball     ball;
static Windmill windmill;
static float    timeOfDay  = 0.45f; // start at midday
static float    todSpeed   = 0.005f; // speed of time passage (manual step)
static bool     lampsOn    = false;
static int      currentHole = 0;
static int      scores[18]  = {};
static bool     aimMode     = false;
static bool     otherSideView = false;

// Global meshes
static Mesh mSphere, mCylinder, mBox, mCone, mQuad, mTorus;

// ─── Collision (AABB fairway walls) ──────────────────────────────────────────
static bool reflectBall(Ball& b, const HoleInfo& h){
    // Fairway AABB (local, then rotate)
    // Simplified: treat fairway as rectangle in XZ
    float hw = h.wid * 0.5f;
    float hl = h.len * 0.5f;
    // transform ball pos to hole-local space
    float dx = b.pos.x - h.tee.x;
    float dz = b.pos.z - h.tee.z;
    // rotate by -h.rot
    float lx =  dx*cosf(-h.rot) + dz*sinf(-h.rot);
    float lz = -dx*sinf(-h.rot) + dz*cosf(-h.rot);
    // fairway local: x in [-hw,hw], z in [-0.5, hl+0.5]
    bool hit = false;
    if (lx < -hw){ lx = -hw; b.vel.x = fabsf(b.vel.x); hit=true; }
    if (lx >  hw){ lx =  hw; b.vel.x = -fabsf(b.vel.x); hit=true; }
    if (lz < -1.0f){ lz = -1.0f; hit=true; }
    if (lz > hl+1.0f){ lz = hl+1.0f; hit=true; }
    // back to world
    b.pos.x = h.tee.x + lx*cosf(h.rot) - lz*sinf(h.rot);
    b.pos.z = h.tee.z + lx*sinf(h.rot) + lz*cosf(h.rot);
    return hit;
}

// ─── GLFW callbacks ──────────────────────────────────────────────────────────
static GLFWwindow* gWin = nullptr;

static void mouseMoveCallback(GLFWwindow*, double x, double y){
    if (cam.firstMouse){ cam.lastX=x; cam.lastY=y; cam.firstMouse=false; }
    float dx = (float)(x - cam.lastX) * 0.003f;
    float dy = (float)(y - cam.lastY) * 0.003f;
    cam.lastX = x; cam.lastY = y;

    // Only move camera if right-button held; aim drag uses left-button
    if (glfwGetMouseButton(gWin, GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS){
        cam.yaw   += dx;
        cam.pitch -= dy;
        cam.pitch  = glm::clamp(cam.pitch, -1.4f, 1.4f);
    }
    if (aimMode && glfwGetMouseButton(gWin, GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS){
        ball.dragCur = {(float)x,(float)y};
    }
}

static void mouseButtonCallback(GLFWwindow*, int btn, int action, int){
    if (!aimMode) return;
    if (btn==GLFW_MOUSE_BUTTON_LEFT){
        if (action==GLFW_PRESS){
            double x,y; glfwGetCursorPos(gWin,&x,&y);
            ball.dragStart = ball.dragCur = {(float)x,(float)y};
        }
        if (action==GLFW_RELEASE && ball.active){
            // Fire!
            vec2 drag = ball.dragStart - ball.dragCur; // drag BACK = fire forward
            float p = glm::length(drag);
            if (p > 5.0f){
                p = glm::min(p, 200.0f);
                // Camera forward projected onto XZ
                vec3 fwd = {sinf(cam.yaw), 0, -cosf(cam.yaw)};
                vec3 right = {cosf(cam.yaw), 0, sinf(cam.yaw)};
                // Map screen drag to world direction
                vec3 dir = glm::normalize(
                    fwd   * (drag.y / glm::length(drag)) +
                    right * (drag.x / glm::length(drag))
                );
                float force = p * 0.08f;
                force = glm::min(force, 12.0f);
                ball.vel    = dir * force;
                ball.moving = true;
                ball.strokes++;
                aimMode = false;
            }
        }
    }
}

static void keyCallback(GLFWwindow* win, int key, int, int action, int){
    if (action == GLFW_PRESS){
        switch(key){
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(win,true); break;
        case GLFW_KEY_O: cam.ortho = !cam.ortho; break;
        case GLFW_KEY_G:
            if (ball.active && !ball.moving){
                aimMode = !aimMode;
            }
            break;
        case GLFW_KEY_SPACE:
            // Spawn ball at current hole tee
            ball.pos    = HOLES[currentHole].tee + vec3(0, ball.radius, 0);
            ball.vel    = {0,0,0};
            ball.active = true;
            ball.moving = false;
            ball.inHole = false;
            ball.strokes= 0;
            aimMode     = false;
            break;
        case GLFW_KEY_RIGHT_BRACKET: // next hole
            currentHole = (currentHole+1)%18;
            ball.active = false;
            aimMode = false;
            break;
        case GLFW_KEY_LEFT_BRACKET: // prev hole
            currentHole = (currentHole+17)%18;
            ball.active = false;
            aimMode = false;
            break;
        // Time of day manual
        case GLFW_KEY_KP_ADD:
            timeOfDay = fmodf(timeOfDay + 0.05f, 1.0f); break;
        case GLFW_KEY_KP_SUBTRACT:
            timeOfDay = fmodf(timeOfDay + 0.95f, 1.0f); break;
        // Auto-advance time toggle
        case GLFW_KEY_T:
            todSpeed = (todSpeed > 0.0f) ? 0.0f : 0.002f; break;
        case GLFW_KEY_V: 
            otherSideView = !otherSideView;
            if (otherSideView){
                cam.pos   = {0,18,-28};
                cam.yaw   = 180;
                cam.pitch = -0.55f;
            } else {
                cam.pos   = {0,18,28};
                cam.yaw   = 0;
                cam.pitch = -0.55f;
            }
            break;
        case GLFW_KEY_R:
            // Reset camera
            cam.pos   = {0,18,28};
            cam.yaw   = 0;
            cam.pitch = -0.55f;
            otherSideView = false;
            break;
        }
    }
}

// ─── Drawing helpers ──────────────────────────────────────────────────────────
static GLuint gProg;

static void setUniforms(float t, const mat4& vp){
    glUniform1f(glGetUniformLocation(gProg,"uTime"),       t);
    glUniform1f(glGetUniformLocation(gProg,"uTimeOfDay"),  timeOfDay);
    glUniform1f(glGetUniformLocation(gProg,"uAmbient"),    0.28f);

    // Sun direction cycles with time of day
    float sunAngle = timeOfDay * 2.0f * PI;
    vec3 sunDir = glm::normalize(vec3(cosf(sunAngle), sinf(sunAngle) * 0.8f + 0.2f, -0.5f));
    glUniform3fv(glGetUniformLocation(gProg,"uLightDir"),  1, glm::value_ptr(sunDir));

    // Sun color: white at noon, orange at dawn/dusk, dim at night
    vec3 sunCol = {1.0f,0.95f,0.85f};
    if (timeOfDay < 0.3f || timeOfDay > 0.75f) sunCol = {0.1f,0.08f,0.06f};
    else if (timeOfDay < 0.4f)  sunCol = mix(vec3{1.0f,0.5f,0.2f}, vec3{1.0f,0.95f,0.85f}, (timeOfDay-0.3f)/0.1f);
    else if (timeOfDay > 0.65f) sunCol = mix(vec3{1.0f,0.95f,0.85f}, vec3{1.0f,0.5f,0.2f}, (timeOfDay-0.65f)/0.1f);
    glUniform3fv(glGetUniformLocation(gProg,"uLightColor"),1,glm::value_ptr(sunCol));

    glUniform3fv(glGetUniformLocation(gProg,"uCamPos"),1,glm::value_ptr(cam.pos));

    // Lamps
    lampsOn = (timeOfDay < 0.28f || timeOfDay > 0.72f);
    glUniform1i(glGetUniformLocation(gProg,"uLampsOn"), lampsOn?1:0);
    glUniform1i(glGetUniformLocation(gProg,"uLampCount"), LAMP_COUNT);
    // Lamp positions are at top of post (y=3.8)
    static vec3 lampWorldPos[32];
    for (int i=0;i<LAMP_COUNT;i++)
        lampWorldPos[i] = LAMP_POSITIONS[i] + vec3(0,3.8f,0);
    glUniform3fv(glGetUniformLocation(gProg,"uLampPos"), LAMP_COUNT, glm::value_ptr(lampWorldPos[0]));
    vec3 lampCol = {1.0f,0.92f,0.65f};
    glUniform3fv(glGetUniformLocation(gProg,"uLampColor"),1,glm::value_ptr(lampCol));
}

// Draw a single hole (fairway + tee + cup)
static void drawHole(const HoleInfo& h, const mat4& vp, float t, int hIdx){
    // Fairway quad
    {
        mat4 m = glm::translate(mat4(1), vec3(h.tee.x, 0, h.tee.z) + vec3(0,0,0));
        // move to midpoint between tee and cup
        vec3 mid = (h.tee + h.cup) * 0.5f;
        mid.y = 0;
        m = glm::translate(mat4(1), mid);
        m = glm::rotate(m, h.rot, vec3(0,1,0));
        m = glm::scale(m, vec3(h.wid, 1, h.len));
        draw(gProg, mQuad, m, vp, 0); // green
    }
    // Tee box
    {
        mat4 m = glm::translate(mat4(1), h.tee);
        m = glm::rotate(m, h.rot, vec3(0,1,0));
        m = glm::scale(m, vec3(h.wid*0.9f, 1, 1.0f));
        draw(gProg, mQuad, m, vp, 1); // tee box
    }
    // Border walls (left, right)
    {
        // Compute world-space sides
        vec3 mid = (h.tee + h.cup) * 0.5f;
        vec3 side = {cosf(h.rot), 0, sinf(h.rot)};
        vec3 fwdN = {-sinf(h.rot), 0, cosf(h.rot)};
        float hw = h.wid*0.5f + 0.1f;
        float wallH = 0.4f;
        for (int s=-1; s<=1; s+=2){
            vec3 wp = mid + side*(float)s*hw + vec3(0,wallH*0.5f,0);
            mat4 m = glm::translate(mat4(1), wp);
            m = glm::rotate(m, h.rot, vec3(0,1,0));
            m = glm::scale(m, vec3(0.2f, wallH, h.len+0.2f));
            draw(gProg, mBox, m, vp, 8);
        }
        // End wall
        vec3 ep = h.cup + fwdN*0.6f + vec3(0,wallH*0.5f,0);
        mat4 m = glm::translate(mat4(1), ep);
        m = glm::rotate(m, h.rot, vec3(0,1,0));
        m = glm::scale(m, vec3(h.wid+0.4f, wallH, 0.2f));
        draw(gProg, mBox, m, vp, 8);
    }
    // Cup hole (torus + black disc)
    {
        mat4 m = glm::translate(mat4(1), h.cup + vec3(0,0.01f,0));
        m = glm::scale(m, vec3(0.32f,0.1f,0.32f));
        draw(gProg, mTorus, m, vp, 19);
        // black disc inside
        mat4 md = glm::translate(mat4(1), h.cup + vec3(0,0.001f,0));
        md = glm::scale(md, vec3(0.30f,1.f,0.30f));
        draw(gProg, mQuad, md, vp, 19);
    }
    // Flag pole + flag
    {
        vec3 fp = h.cup + vec3(0.3f,0,0);
        // Pole
        mat4 m = glm::translate(mat4(1), fp);
        m = glm::scale(m, vec3(0.04f, 2.0f, 0.04f));
        draw(gProg, mCylinder, m, vp, 16);
        // Flag (small quad)
        mat4 mf = glm::translate(mat4(1), fp+vec3(0.25f, 1.8f, 0));
        mf = glm::rotate(mf, PI*0.5f, vec3(0,1,0));
        mf = glm::scale(mf, vec3(0.6f,1,0.4f));
        draw(gProg, mQuad, mf, vp, 17);
    }
    // Hole number label (skip — no text renderer)
}

// Draw palm tree at given position, scale
static void drawPalmTree(vec3 pos, float scale, const mat4& vp){
    // Trunk (cylinder, slightly curved by stacking segments)
    int segs = 5;
    for (int i=0;i<segs;i++){
        float y0 = (float)i*1.2f*scale;
        float y1 = (float)(i+1)*1.2f*scale;
        float lean = (float)i * 0.04f;
        vec3 p = pos + vec3(lean*scale, y0, 0);
        mat4 m = glm::translate(mat4(1),p);
        m = glm::rotate(m, lean*0.3f, vec3(0,0,1));
        m = glm::scale(m, vec3(0.18f*scale, (y1-y0), 0.18f*scale));
        draw(gProg, mCylinder, m, vp, 11);
    }
    // Crown of fronds (8 leaves radiating from top)
    vec3 top = pos + vec3(segs*0.04f*scale, segs*1.2f*scale, 0);
    int leafCount = 8;
    for (int i=0;i<leafCount;i++){
        float ang = 2*PI*(float)i/leafCount;
        float tilt = 0.7f; // radians from vertical
        mat4 m = glm::translate(mat4(1),top);
        m = glm::rotate(m, ang, vec3(0,1,0));
        m = glm::rotate(m, tilt, vec3(1,0,0));
        m = glm::scale(m, vec3(0.25f*scale, 1.0f, 1.5f*scale));
        draw(gProg, mQuad, m, vp, 12);
        // Mirrored
        m = glm::translate(mat4(1),top);
        m = glm::rotate(m, ang, vec3(0,1,0));
        m = glm::rotate(m, -tilt, vec3(1,0,0));
        m = glm::scale(m, vec3(0.25f*scale, 1.0f, 1.5f*scale));
        draw(gProg, mQuad, m, vp, 12);
    }
}

// Draw boulder at position
static void drawBoulder(vec3 pos, float r, const mat4& vp){
    mat4 m = glm::translate(mat4(1),pos);
    m = glm::scale(m, vec3(r, r*0.75f, r));
    draw(gProg, mSphere, m, vp, 10);
}

static void drawRocks(const mat4& vp){

    drawBoulder({-25,0.5f,-10}, 1.0f, vp);
    drawBoulder({-23,0.4f,-12}, 0.7f, vp);
    drawBoulder({-20,0.6f,-8},  1.2f, vp);

    drawBoulder({20,0.5f,-5},  1.1f, vp);
    drawBoulder({22,0.4f,-7},  0.8f, vp);

    drawBoulder({0,0.5f,0},    1.3f, vp);

    drawBoulder({6,0.5f,8},    1.0f, vp);

    drawBoulder({18,0.5f,-14}, 1.2f, vp);
}
static void drawFlower(vec3 pos, float scale, const mat4& vp)
{

    // stem of the flower
    mat4 stem = glm::translate(mat4(1), pos + vec3(0, 0.25f*scale, 0));
    stem = glm::scale(stem, vec3(0.04f*scale, 0.5f*scale, 0.04f*scale));
    draw(gProg, mCylinder, stem, vp, 11);

    // flower head
    vec3 head = pos + vec3(0, 0.75f*scale, 0);

    // petals of the flower
    for (int i = 0; i < 6; i++){
        float ang = 2.0f * PI * i / 6.0f;

        vec3 petalPos = head + vec3(
            cosf(ang) * 0.22f * scale,
            0,
            sinf(ang) * 0.22f * scale
        );

        mat4 petal = glm::translate(mat4(1), petalPos);
        petal = glm::scale(petal, vec3(0.18f*scale, 0.08f*scale, 0.18f*scale));
        draw(gProg, mSphere, petal, vp, 17);
    }

    // center of the flowers
    mat4 center = glm::translate(mat4(1), head);
    center = glm::scale(center, vec3(0.12f*scale));
    draw(gProg, mSphere, center, vp, 15);
}

static void drawFlowers(const mat4& vp){
    drawFlower({-6,0,-8}, 2.0f, vp);
    drawFlower({-5,0,-8}, 1.8f, vp);
    drawFlower({8,0,-10}, 2.0f, vp);
    drawFlower({-10,0,10}, 2.0f, vp);
    drawFlower({12,0,12}, 2.0f, vp);
}

// Draw lamp post
static void drawLampPost(vec3 pos, const mat4& vp){
    // Pole
    {
        mat4 m = glm::translate(mat4(1),pos);
        m = glm::scale(m, vec3(0.10f, 4.0f, 0.10f));
        draw(gProg, mCylinder, m, vp, 13);
    }
    // Arm
    {
        mat4 m = glm::translate(mat4(1), pos + vec3(0,4.0f,0));
        m = glm::scale(m, vec3(0.8f, 0.08f, 0.08f));
        draw(gProg, mBox, m, vp, 13);
    }
    // Lamp head (small sphere, emissive)
    {
        mat4 m = glm::translate(mat4(1), pos + vec3(0.4f, 4.0f, 0));
        m = glm::scale(m, vec3(0.25f,0.25f,0.25f));
        draw(gProg, mSphere, m, vp, 14);
    }
}

// Draw windmill at the windmill pos
static void drawWindmill(const Windmill& wm, const mat4& vp){
    vec3 p = wm.pos;

    // Body — smooth cylinder tower (tapered slightly)
    {
        // Base wide section
        mat4 m = glm::translate(mat4(1), p);
        m = glm::scale(m, vec3(1.1f, 2.0f, 1.1f));
        draw(gProg, mCylinder, m, vp, 7);
        // Mid section
        mat4 m2 = glm::translate(mat4(1), p + vec3(0,2,0));
        m2 = glm::scale(m2, vec3(0.90f, 2.0f, 0.90f));
        draw(gProg, mCylinder, m2, vp, 7);
        // Upper section
        mat4 m3 = glm::translate(mat4(1), p + vec3(0,4,0));
        m3 = glm::scale(m3, vec3(0.72f, 1.5f, 0.72f));
        draw(gProg, mCylinder, m3, vp, 7);
        // Conical cap
        mat4 mc = glm::translate(mat4(1), p + vec3(0,5.5f,0));
        mc = glm::scale(mc, vec3(0.85f, 1.2f, 0.85f));
        draw(gProg, mCone, mc, vp, 7);
    }

    // Blade hub
    vec3 hub = p + vec3(0, 4.8f, -0.8f);
    {
        mat4 m = glm::translate(mat4(1), hub);
        m = glm::scale(m, vec3(0.22f,0.22f,0.22f));
        draw(gProg, mSphere, m, vp, 13);
    }

    // 4 cross blades, each is a long thin quad
    for (int b=0;b<4;b++){
        float angle = wm.rot + b * PI * 0.5f;
        float bx = cosf(angle) * 1.6f;
        float by = sinf(angle) * 1.6f;
        vec3 bpos = hub + vec3(bx, by, 0);
        mat4 m = glm::translate(mat4(1), bpos);
        m = glm::rotate(m, angle, vec3(0,0,1));
        m = glm::scale(m, vec3(0.55f, 3.2f, 0.12f));
        draw(gProg, mBox, m, vp, 18);
    }
}

static void drawChair(vec3 pos, float rot, const mat4& vp)
{
    mat4 base = glm::translate(mat4(1), pos);
    base = glm::rotate(base, rot, vec3(0, 1, 0));

    // Seat
    mat4 seat = base;
    seat = glm::translate(seat, vec3(0, 0.55f, 0));
    seat = glm::scale(seat, vec3(0.9f, 0.14f, 0.9f));
    draw(gProg, mBox, seat, vp, 17);

    // Backrest
    mat4 back = base;
    back = glm::translate(back, vec3(0, 1.05f, 0.42f));
    back = glm::scale(back, vec3(0.9f, 1.0f, 0.12f));
    draw(gProg, mBox, back, vp, 17);

    // Legs
    mat4 leg1 = base;
    leg1 = glm::translate(leg1, vec3(-0.34f, 0.27f, -0.34f));
    leg1 = glm::scale(leg1, vec3(0.10f, 0.55f, 0.10f));
    draw(gProg, mBox, leg1, vp, 13);

    mat4 leg2 = base;
    leg2 = glm::translate(leg2, vec3(0.34f, 0.27f, -0.34f));
    leg2 = glm::scale(leg2, vec3(0.10f, 0.55f, 0.10f));
    draw(gProg, mBox, leg2, vp, 13);

    mat4 leg3 = base;
    leg3 = glm::translate(leg3, vec3(-0.34f, 0.27f, 0.34f));
    leg3 = glm::scale(leg3, vec3(0.10f, 0.55f, 0.10f));
    draw(gProg, mBox, leg3, vp, 13);

    mat4 leg4 = base;
    leg4 = glm::translate(leg4, vec3(0.34f, 0.27f, 0.34f));
    leg4 = glm::scale(leg4, vec3(0.10f, 0.55f, 0.10f));
    draw(gProg, mBox, leg4, vp, 13);
}



static void drawTable(vec3 pos, const mat4& vp){

    // Top
    mat4 top = glm::translate(mat4(1), pos + vec3(0, 1, 0));
    top = glm::scale(top, vec3(2, 0.2f, 2));
    draw(gProg, mBox, top, vp, 17);

    // Legs
    mat4 leg1 = glm::translate(mat4(1), pos + vec3(-0.8f, 0.5f, -0.8f));
    leg1 = glm::scale(leg1, vec3(0.1f, 1, 0.1f));
    draw(gProg, mBox, leg1, vp, 13);

    mat4 leg2 = glm::translate(mat4(1), pos + vec3(0.8f, 0.5f, -0.8f));
    leg2 = glm::scale(leg2, vec3(0.1f, 1, 0.1f));
    draw(gProg, mBox, leg2, vp, 13);

    mat4 leg3 = glm::translate(mat4(1), pos + vec3(-0.8f, 0.5f, 0.8f));
    leg3 = glm::scale(leg3, vec3(0.1f, 1, 0.1f));
    draw(gProg, mBox, leg3, vp, 13);

    mat4 leg4 = glm::translate(mat4(1), pos + vec3(0.8f, 0.5f, 0.8f));
    leg4 = glm::scale(leg4, vec3(0.1f, 1, 0.1f));
    draw(gProg, mBox, leg4, vp, 13);
}

// Draw restaurant area in center
static void drawRestaurant(const mat4& vp){
    // Floor
    {
        mat4 m = glm::translate(mat4(1), vec3(0,0.01f,4));
        m = glm::scale(m, vec3(16, 1, 12));
        draw(gProg, mQuad, m, vp, 9);
    }
    // 4 walls
    float wx=8, wz=6, wh=3;
    // Front wall
    {mat4 m=glm::translate(mat4(1),{0,wh*0.5f,4-wz}); m=glm::scale(m,{wx*2,wh,0.3f}); draw(gProg,mBox,m,vp,8);}
    // Back wall
    {mat4 m=glm::translate(mat4(1),{0,wh*0.5f,4+wz}); m=glm::scale(m,{wx*2,wh,0.3f}); draw(gProg,mBox,m,vp,8);}
    // Left wall
    {mat4 m=glm::translate(mat4(1),{-wx,wh*0.5f,4}); m=glm::scale(m,{0.3f,wh,wz*2}); draw(gProg,mBox,m,vp,8);}
    // Right wall
    {mat4 m=glm::translate(mat4(1),{wx,wh*0.5f,4}); m=glm::scale(m,{0.3f,wh,wz*2}); draw(gProg,mBox,m,vp,8);}

drawTable({-6.0f,0,-3.8f}, vp);
drawChair({-7.3f,0,-3.8f}, -1.57f, vp);
drawChair({-4.7f,0,-3.8f},  1.57f, vp);
drawChair({-6.0f,0,-5.1f},  3.14f, vp);
drawChair({-6.0f,0,-2.5f},  0.0f,  vp);


drawTable({6.0f,0,-3.8f}, vp);
drawChair({4.7f,0,-3.8f}, -1.57f, vp);
drawChair({7.3f,0,-3.8f},  1.57f, vp);
drawChair({6.0f,0,-5.1f},  3.14f, vp);
drawChair({6.0f,0,-2.5f},  0.0f,  vp);

    // Roof
    {
        mat4 m=glm::translate(mat4(1),{0,wh+0.4f,4});
        m=glm::scale(m,{wx*2+1,0.3f,wz*2+1});
        draw(gProg,mBox,m,vp,20);
    }
}

// Draw walkway between two points
static void drawWalkway(vec3 a, vec3 b, float w, const mat4& vp){
    vec3 mid = (a+b)*0.5f;
    float len = glm::length(b-a);
    float ang = atan2f(b.x-a.x, b.z-a.z);
    mat4 m = glm::translate(mat4(1),mid+vec3(0,0.005f,0));
    m = glm::rotate(m, ang, vec3(0,1,0));
    m = glm::scale(m, vec3(w, 1, len));
    draw(gProg, mQuad, m, vp, 3);
}

// ─── Skybox geometry ─────────────────────────────────────────────────────────
static Mesh mSkybox;
static GLuint skyProg;

static Mesh makeSkyboxCube(){
    float v[] = {
       -1, 1,-1, -1,-1,-1,  1,-1,-1,  1,-1,-1,  1, 1,-1, -1, 1,-1,
       -1,-1, 1, -1,-1,-1, -1, 1,-1, -1, 1,-1, -1, 1, 1, -1,-1, 1,
        1,-1,-1,  1,-1, 1,  1, 1, 1,  1, 1, 1,  1, 1,-1,  1,-1,-1,
       -1,-1, 1, -1, 1, 1,  1, 1, 1,  1, 1, 1,  1,-1, 1, -1,-1, 1,
       -1, 1,-1,  1, 1,-1,  1, 1, 1,  1, 1, 1, -1, 1, 1, -1, 1,-1,
       -1,-1,-1, -1,-1, 1,  1,-1,-1,  1,-1,-1, -1,-1, 1,  1,-1, 1,
    };
    GLuint vao,vbo;
    glGenVertexArrays(1,&vao);
    glGenBuffers(1,&vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    Mesh m; m.vao=vao; m.vbo=vbo; m.ebo=0; m.count=36;
    return m;
}

// ─── Main ────────────────────────────────────────────────────────────────────
int main(){
    if (!glfwInit()){ puts("glfwInit failed"); return 1; }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    gWin = glfwCreateWindow(WIN_W, WIN_H, "COS344 HA - Mini Golf Course", NULL, NULL);
    if (!gWin){ glfwTerminate(); puts("Window failed"); return 1; }
    glfwMakeContextCurrent(gWin);
    glewExperimental = true;
    if (glewInit()!=GLEW_OK){ glfwTerminate(); puts("GLEW failed"); return 1; }

    glfwSetKeyCallback(gWin, keyCallback);
    glfwSetCursorPosCallback(gWin, mouseMoveCallback);
    glfwSetMouseButtonCallback(gWin, mouseButtonCallback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.02f,0.03f,0.08f,1);

    gProg   = LoadShaders("golf_vert.glsl","golf_frag.glsl");
    skyProg = LoadShaders("sky_vert.glsl", "sky_frag.glsl");

    // Build meshes
    mSphere   = makeSphere(18,32);
    mCylinder = makeCylinder(0.5f,1.0f,24);
    mBox      = makeBox();
    mCone     = makeCone(0.5f,1.0f,24);
    mQuad     = makeQuad();
    mTorus    = makeTorus(1.0f,0.05f,32,12);
    mSkybox   = makeSkyboxCube();

    // Spawn ball at hole 0
    ball.pos    = HOLES[0].tee + vec3(0, ball.radius, 0);
    ball.holeIdx= 0;
    ball.active = true;

    double prevTime = glfwGetTime();

    while (!glfwWindowShouldClose(gWin)){
        double now = glfwGetTime();
        float  dt  = (float)(now - prevTime);
        prevTime   = now;

        // Time of day
        timeOfDay = fmodf(timeOfDay + todSpeed*dt, 1.0f);

        // Windmill blades
        windmill.rot = fmodf(windmill.rot + dt * 0.8f, 2*PI);

        // Ball physics
        if (ball.active && ball.moving){
            ball.update(dt);
            // Check hole completion
            if (ball.nearCup(HOLES[ball.holeIdx].cup)){
                scores[ball.holeIdx] = ball.strokes;
                ball.inHole = true;
                ball.moving = false;
                ball.active = false;
                printf("Hole %d complete! %d strokes (par %d)\n",
                       ball.holeIdx+1, ball.strokes, HOLES[ball.holeIdx].par);
            }
            // Soft wall reflection
            reflectBall(ball, HOLES[ball.holeIdx]);
        }

        // Camera movement (WASD+QE)
        float spd = cam.speed * dt;
        vec3 fwd = {sinf(cam.yaw)*cosf(cam.pitch), sinf(cam.pitch), -cosf(cam.yaw)*cosf(cam.pitch)};
        vec3 right = glm::normalize(glm::cross(fwd, vec3(0,1,0)));
        if (glfwGetKey(gWin,GLFW_KEY_W)==GLFW_PRESS) cam.pos += fwd  * spd;
        if (glfwGetKey(gWin,GLFW_KEY_S)==GLFW_PRESS) cam.pos -= fwd  * spd;
        if (glfwGetKey(gWin,GLFW_KEY_A)==GLFW_PRESS) cam.pos -= right* spd;
        if (glfwGetKey(gWin,GLFW_KEY_D)==GLFW_PRESS) cam.pos += right* spd;
        if (glfwGetKey(gWin,GLFW_KEY_Q)==GLFW_PRESS) cam.pos.y -= spd;
        if (glfwGetKey(gWin,GLFW_KEY_E)==GLFW_PRESS) cam.pos.y += spd;

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        float asp = (float)WIN_W/WIN_H;
        mat4 view = cam.view();
        mat4 proj = cam.proj(asp);
        mat4 vp   = proj * view;

        // ── Skybox (depth test to LEQUAL, no depth write) ──
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        glUseProgram(skyProg);
        // Remove translation from view
        mat4 skyView = mat4(mat3(view));
        mat4 skyVP   = proj * skyView;
        glUniformMatrix4fv(glGetUniformLocation(skyProg,"uVP"),1,GL_FALSE,glm::value_ptr(skyVP));
        glUniform1f(glGetUniformLocation(skyProg,"uTimeOfDay"),timeOfDay);
        glBindVertexArray(mSkybox.vao);
        glDrawArrays(GL_TRIANGLES,0,36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        // ── Course rendering ──
        glUseProgram(gProg);
        float t = (float)now;
        setUniforms(t, vp);

        // Ground plane (large base)
        {
            mat4 m = glm::scale(mat4(1), vec3(80,1,80));
            draw(gProg, mQuad, m, vp, 2); // rough
        }

        // All 18 holes
        for (int h=0;h<18;h++)
            drawHole(HOLES[h], vp, t, h);

        // Walkways connecting holes (path between cup of hole N → tee of hole N+1)
        for (int h=0;h<17;h++)
            drawWalkway(HOLES[h].cup + vec3(0,0.01f,0),
                        HOLES[h+1].tee + vec3(0,0.01f,0),
                        1.8f, vp);

        // Restaurant area
        drawRestaurant(vp);

        // Lamp posts
        for (int i=0;i<LAMP_COUNT;i++)
            drawLampPost(LAMP_POSITIONS[i], vp);

        // Windmill (hole 7)
        drawWindmill(windmill, vp);

        // Palm trees (scattered around course)
        static vec3 palmPos[] = {
            {-26,0,-20},{-22,0,-20},{-20,0,-14},
            {  2,0,-20},{  8,0,-20},{ 14,0,-20},
            { 20,0,-20},{ 26,0,-20},{-26,0, 20},
            {-22,0, 20},{ -8,0, 20},{  4,0, 20},
            { 10,0, 20},{ 20,0, 20},{ 26,0, 20},
            {-14,0,  2},{ 14,0,  2},{-14,0, -2},
        };
        for (auto& pp : palmPos)
            drawPalmTree(pp, 1.0f, vp);

        drawRocks(vp);
        drawFlowers(vp);
        // Smaller palms
        drawPalmTree({-4,0,-4},0.7f,vp);
        drawPalmTree({ 4,0,-4},0.7f,vp);

        // Boulders
        static vec3 boulderPos[] = {
            {-22,0,-6},{ 22,0,-6},{-22,0, 6},{ 22,0, 6},
            {-10,0,-14},{ 10,0,-14},{-10,0, 14},{ 10,0, 14},
            {  0,0,-24},{  0,0, 24},{-26,0,  0},{ 26,0,  0},
        };
        static float boulderR[] = {0.8f,1.2f,0.6f,1.0f,0.7f,0.9f,1.1f,0.8f,1.3f,0.7f,0.9f,1.1f};
        for (int i=0;i<12;i++)
            drawBoulder(boulderPos[i]+vec3(0,boulderR[i]*0.4f,0), boulderR[i], vp);

        // Golf ball
        if (ball.active){
            mat4 m = glm::translate(mat4(1), ball.pos);
            m = glm::scale(m, vec3(ball.radius*2));
            draw(gProg, mSphere, m, vp, 15);
        }

        // Aim indicator (line from ball showing power+direction)
        // We approximate with a thin box
        if (aimMode && ball.active){
            vec2 drag = ball.dragStart - ball.dragCur;
            if (glm::length(drag) > 3.0f){
                vec3 fwdAim = {sinf(cam.yaw), 0, -cosf(cam.yaw)};
                vec3 rgtAim = {cosf(cam.yaw), 0,  sinf(cam.yaw)};
                vec3 dir = glm::normalize(
                    fwdAim*(drag.y/glm::length(drag)) +
                    rgtAim*(drag.x/glm::length(drag)));
                float pw = glm::min(glm::length(drag)/200.0f, 1.0f);
                float len = pw * 4.0f;
                float ang = atan2f(dir.x, dir.z);
                vec3 midPt = ball.pos + dir*(len*0.5f);
                mat4 m = glm::translate(mat4(1),midPt);
                m = glm::rotate(m, ang, vec3(0,1,0));
                m = glm::scale(m, vec3(0.06f, 0.06f, len));
                draw(gProg, mBox, m, vp, 17); // red arrow
            }
        }

        glfwSwapBuffers(gWin);
        glfwPollEvents();
    }

    // Cleanup
    freeMesh(mSphere); freeMesh(mCylinder); freeMesh(mBox);
    freeMesh(mCone);   freeMesh(mQuad);     freeMesh(mTorus);
    glDeleteBuffers(1,&mSkybox.vbo);
    glDeleteVertexArrays(1,&mSkybox.vao);
    glDeleteProgram(gProg);
    glDeleteProgram(skyProg);
    glfwDestroyWindow(gWin);
    glfwTerminate();
    return 0;
}
