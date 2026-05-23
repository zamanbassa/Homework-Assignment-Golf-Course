/* ============================================================
   COS344 HA — Mini Golf  (clean base: 1 hole, 1 restaurant)
   Student: u14439141  Team: Ray Tracers

   Controls:
     WASD       fly camera
     Q / E      camera down / up
     Scroll     zoom in / out
     Right-drag smooth mouse look
     Space      spawn ball at tee
     G          toggle aim mode (ball must be still)
     LMB-drag   aim + power (in aim mode)
     Release    fire
     O          ortho / perspective
     R          reset camera
     KP+ / KP-  step time of day
     T          toggle auto time
     Escape     quit
   ============================================================ */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

#include "constants.h"
#include "Meshes/Mesh.h"
#include "Meshes/MeshFactory.h"
#include "Camera.h"
#include "Ball.h"
#include "Holes.h"

using namespace std;
using namespace glm;

// ─── Camera ──────────────────────────────────────────────────────────────────

MeshFactory meshFactory;

// ─── Globals ──────────────────────────────────────────────────────────────────
GLuint gProg, skyProg;
Mesh mQuad, mBox, mSphere, mCylinder, mTorus, mTrap, mSkybox, mCircle;
Mesh mH3Wall1, mH3Wall2;                                   // hole 3 arc walls (south loop, north loop)
Mesh mH4Floor, mH4WallIn, mH4WallOut;                      // hole 4 banana meshes
Mesh mH5Floor;                                             // hole 5 pentagon floor
Mesh mH7FloorArc1, mH7FloorArc2;                           // hole 7 S-shape floors
Mesh mH7WallA1In, mH7WallA1Out, mH7WallA2In, mH7WallA2Out; // hole 7 S-shape walls
Mesh mH8Floor;                                             // hole 8 triangle floor
static GLFWwindow *gWin = nullptr;
static float timeOfDay = 0.45f; // 0=midnight, 0.5=noon
static float todSpeed = 0.002f; // auto advance (full cycle ≈ 8 min)

// ─── Camera ──────────────────────────────────────────────────────────────────
Camera cam;

// ─── Ball ─────────────────────────────────────────────────────────────────────

Ball ball;
// ─── Aim state ────────────────────────────────────────────────────────────────
static bool aimMode = false;
static vec3 aimTarget = {0, 0, 0};

// ─── Mouse state ─────────────────────────────────────────────────────────────
static bool rmbDown = false;
static bool skipFirst = true;
static double lastMX = 0, lastMY = 0;

// Unproject mouse screen coords to the y=0 ground plane
static vec3 mouseToGround(double mx, double my)
{
    float asp = (float)WIN_W / WIN_H;
    float nx = (float)(2.0 * mx / WIN_W - 1.0);
    float ny = (float)(1.0 - 2.0 * my / WIN_H);

    vec4 rayEye = glm::inverse(cam.proj(asp)) * vec4(nx, ny, -1.f, 1.f);
    rayEye = {rayEye.x, rayEye.y, -1.f, 0.f};
    vec3 rayWorld = glm::normalize(vec3(glm::inverse(cam.view()) * rayEye));

    if (fabsf(rayWorld.y) < 0.001f)
        return ball.getPos();
    float t = -cam.getPos().y / rayWorld.y;
    if (t < 0.f)
        return ball.getPos();
    return cam.getPos() + rayWorld * t;
}

// Clamp raw ground point to MAX_AIM distance from ball, then store in aimTarget
static void updateAimTarget(double mx, double my)
{
    vec3 raw = mouseToGround(mx, my);
    vec3 diff = {raw.x - ball.getPos().x, 0.f, raw.z - ball.getPos().z};
    float d = glm::length(diff);
    if (d > 0.001f && d > MAX_AIM)
        aimTarget = {ball.getPos().x + diff.x / d * MAX_AIM, 0.f, ball.getPos().z + diff.z / d * MAX_AIM};
    else
        aimTarget = {raw.x, 0.f, raw.z};
}

// ─── Callbacks ───────────────────────────────────────────────────────────────
static void cbScroll(GLFWwindow *, double, double dy)
{
    cam.setFOV(glm::clamp(cam.getFOV() - (float)dy * 2.5f, 10.f, 95.f));
}

static void cbMouseBtn(GLFWwindow *, int btn, int act, int)
{
    if (btn == GLFW_MOUSE_BUTTON_RIGHT)
    {
        rmbDown = (act == GLFW_PRESS);
        skipFirst = true;
        if (rmbDown)
            glfwSetInputMode(gWin, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(gWin, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (btn == GLFW_MOUSE_BUTTON_LEFT && aimMode)
    {
        if (act == GLFW_PRESS)
        {
            double x, y;
            glfwGetCursorPos(gWin, &x, &y);
            updateAimTarget(x, y);
        }
        if (act == GLFW_RELEASE && ball.getActive() && !ball.getMoving())
        {
            vec3 diff = {aimTarget.x - ball.getPos().x, 0, aimTarget.z - ball.getPos().z};
            float dist = glm::length(diff);
            if (dist > 0.05f)
            {
                vec3 dir = diff / dist;
                float power = (dist / MAX_AIM) * 28.f; // full line = travels ~full fairway
                power = glm::max(power, 0.5f);
                ball.setVel(dir * power);
                ball.setMoving(true);
                ball.setStrokes(ball.getStrokes() + 1);
                aimMode = false;
            }
        }
    }
}

static void cbMouseMove(GLFWwindow *, double x, double y)
{
    if (aimMode && glfwGetMouseButton(gWin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        updateAimTarget(x, y);

    if (!rmbDown)
        return;
    if (skipFirst)
    {
        lastMX = x;
        lastMY = y;
        skipFirst = false;
        return;
    }

    double dx = x - lastMX, dy = y - lastMY;
    lastMX = x;
    lastMY = y;
    cam.setYaw(cam.getYaw() + (float)dx * 0.0018f);
    cam.setPitch(cam.getPitch() - (float)dy * 0.0018f);
    cam.setPitch(glm::clamp(cam.getPitch(), -1.4f, 1.4f));
}

static void cbKey(GLFWwindow *w, int key, int, int act, int)
{
    if (act != GLFW_PRESS)
        return;
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(w, true);
        break;
    case GLFW_KEY_O:
        cam.setOrtho(!cam.getOrtho());
        break;
    case GLFW_KEY_R:
        cam.setPos({-33, 25, 55});
        cam.setYaw(0);
        cam.setPitch(-0.50f);
        cam.setFOV(60.f);
        break;
    case GLFW_KEY_G:
        if (ball.getActive() && !ball.getMoving())
            aimMode = !aimMode;
        break;
    case GLFW_KEY_SPACE:
    {
        float sx, sz, teeZ;
        if (gCurrentHole == 1)
        {
            sx = HOLE1_X;
            sz = HOLE1_Z;
            teeZ = sz + 5.5f;
        }
        else if (gCurrentHole == 2)
        {
            sx = HOLE2_X;
            sz = HOLE2_Z;
            teeZ = sz + 5.5f;
        }
        else if (gCurrentHole == 3)
        {
            sx = HOLE3_X;
            sz = HOLE3_Z;
            teeZ = sz + H3_R * 1.9f - 0.5f;
        }
        else if (gCurrentHole == 4)
        {
            sx = HOLE4_CX + (H4_RI + H4_RO) * 0.5f;
            sz = HOLE4_CZ;
            teeZ = sz;
        }
        else if (gCurrentHole == 5)
        {
            sx = HOLE5_CX;
            sz = HOLE5_CZ;
            teeZ = HOLE5_CZ + H5_R - 1.5f;
        }
        else if (gCurrentHole == 6)
        {
            sx = H6_CX;
            sz = H6_TEE_Z;
            teeZ = H6_TEE_Z - 1.0f;
        }
        else if (gCurrentHole == 7)
        {
            sx = H7_X0 + H7_LS + 1.0f;
            sz = H7_Z0 + 1.0f;
            teeZ = H7_Z0 + 1.0f;
        }
        else if (gCurrentHole == 8)
        {
            sx = H8_BX + 0.8f;
            sz = H8_CZ;
            teeZ = H8_CZ;
        }
        else
        {
            sx = H8_BX + 0.8f;
            sz = H8_CZ;
            teeZ = H8_CZ;
        }
        ball.setPos({sx, BALL_R, teeZ});
        ball.setVel({0, 0, 0});
        ball.setActive(true);
        ball.setMoving(false);
        ball.setInHole(false);
        ball.setStrokes(0);
        aimMode = false;
        break;
    }
    case GLFW_KEY_KP_ADD:
        timeOfDay = fmodf(timeOfDay + 0.05f, 1.f);
        break;
    case GLFW_KEY_KP_SUBTRACT:
        timeOfDay = fmodf(timeOfDay + 0.95f, 1.f);
        break;
    case GLFW_KEY_T:
        todSpeed = (todSpeed > 0) ? 0 : 0.002f;
        break;
    case GLFW_KEY_EQUAL:
        cam.setFOV(glm::max(cam.getFOV() - 3.f, 10.f));
        break;
    case GLFW_KEY_MINUS:
        cam.setFOV(glm::min(cam.getFOV() + 3.f, 95.f));
        break;
    }
}

// ─── Set lighting uniforms ────────────────────────────────────────────────────
static void setUniforms(float t, const mat4 &vp)
{
    (void)vp;
    glUniform1f(glGetUniformLocation(gProg, "uTime"), t);
    glUniform1f(glGetUniformLocation(gProg, "uTimeOfDay"), timeOfDay);
    glUniform1f(glGetUniformLocation(gProg, "uAmbient"), 0.30f);

    float sun = timeOfDay * 2 * PI;
    vec3 sdir = glm::normalize(vec3(cosf(sun), sinf(sun) * 0.9f + 0.1f, -0.4f));
    glUniform3fv(glGetUniformLocation(gProg, "uLightDir"), 1, glm::value_ptr(sdir));

    vec3 scol = {1.f, 0.95f, 0.85f};
    if (timeOfDay < 0.27f || timeOfDay > 0.76f)
        scol = {0.07f, 0.05f, 0.04f};
    else if (timeOfDay < 0.37f)
        scol = glm::mix(vec3(1, .5f, .2f), vec3(1, .95f, .85f), (timeOfDay - .27f) / .1f);
    else if (timeOfDay > 0.67f)
        scol = glm::mix(vec3(1, .95f, .85f), vec3(1, .5f, .2f), (timeOfDay - .67f) / .09f);
    glUniform3fv(glGetUniformLocation(gProg, "uLightColor"), 1, glm::value_ptr(scol));
    glUniform3fv(glGetUniformLocation(gProg, "uCamPos"), 1, glm::value_ptr(cam.getPos()));

    // No lamp posts for now
    glUniform1i(glGetUniformLocation(gProg, "uLampsOn"), 0);
    glUniform1i(glGetUniformLocation(gProg, "uLampCount"), 0);
}
// ─── Draw the restaurant (simple block + roof) ────────────────────────────────
static void drawRestaurant(const mat4 &vp)
{
    float cx = 0.f, cz = 0.f;
    float bw = 7.f, bd = 5.5f, bh = 3.2f;

    // Floor
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.01f, cz});
        m = glm::scale(m, {bw, 1, bd});
        draw(mQuad, m, vp, 9);
    }
    // Walls (4 sides)
    // Front
    {
        mat4 m = glm::translate(mat4(1), {cx, bh * .5f, cz - bd * .5f});
        m = glm::scale(m, {bw, .0f + bh, .28f});
        draw(mBox, m, vp, 8);
    }
    // Back
    {
        mat4 m = glm::translate(mat4(1), {cx, bh * .5f, cz + bd * .5f});
        m = glm::scale(m, {bw, bh, .28f});
        draw(mBox, m, vp, 8);
    }
    // Left
    {
        mat4 m = glm::translate(mat4(1), {cx - bw * .5f, bh * .5f, cz});
        m = glm::scale(m, {.28f, bh, bd});
        draw(mBox, m, vp, 8);
    }
    // Right
    {
        mat4 m = glm::translate(mat4(1), {cx + bw * .5f, bh * .5f, cz});
        m = glm::scale(m, {.28f, bh, bd});
        draw(mBox, m, vp, 8);
    }
    // Roof (slightly overhanging, terracotta)
    {
        mat4 m = glm::translate(mat4(1), {cx, bh + .2f, cz});
        m = glm::scale(m, {bw + .6f, .4f, bd + .6f});
        draw(mBox, m, vp, 20);
    }
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main()
{
    if (!glfwInit())
    {
        puts("glfwInit failed");
        return 1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    gWin = glfwCreateWindow(WIN_W, WIN_H, "COS344 HA - Mini Golf (u14439141)", NULL, NULL);
    if (!gWin)
    {
        glfwTerminate();
        puts("Window failed");
        return 1;
    }
    glfwMakeContextCurrent(gWin);
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        puts("GLEW failed");
        return 1;
    }

    glfwSetKeyCallback(gWin, cbKey);
    glfwSetMouseButtonCallback(gWin, cbMouseBtn);
    glfwSetCursorPosCallback(gWin, cbMouseMove);
    glfwSetScrollCallback(gWin, cbScroll);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.01f, 0.02f, 0.06f, 1);

    gProg = LoadShaders("golf_vert.glsl", "golf_frag.glsl");
    skyProg = LoadShaders("sky_vert.glsl", "sky_frag.glsl");

    mQuad = meshFactory.makeQuad();
    mBox = meshFactory.makeBox();
    mSphere = meshFactory.makeSphere();
    mCylinder = meshFactory.makeCylinder();
    mTorus = meshFactory.makeTorus(1.f, 0.05f, 28, 10);
    mTrap = meshFactory.makeTrapezoid();
    mCircle = meshFactory.makeCircle();

    {
        const float GAP = 0.55f;
        // South loop: gap at -PI/2 (pointing toward north loop)
        mH3Wall1 = meshFactory.makeArcWall(H3_R, 0.28f, 0.45f, -PI / 2.0f + GAP, -PI / 2.0f + 2.0f * PI - GAP, 56);
        // North loop: gap at +PI/2 (pointing toward south loop)
        mH3Wall2 = meshFactory.makeArcWall(H3_R, 0.28f, 0.45f, PI / 2.0f + GAP, PI / 2.0f + 2.0f * PI - GAP, 56);
    }

    mH4Floor = meshFactory.makeArcFloor(H4_RI, H4_RO, H4_T0, H4_T1, 52);
    mH4WallIn = meshFactory.makeArcWall(H4_RI, 0.28f, 0.45f, H4_T0, H4_T1, 52);
    mH4WallOut = meshFactory.makeArcWall(H4_RO, 0.28f, 0.45f, H4_T0, H4_T1, 52);
    mH5Floor = meshFactory.makePentagonFloor();

    {
        float ri = H7_RA - H7_FW * 0.5f; // 3.0
        float ro = H7_RA + H7_FW * 0.5f; // 7.0
        // Arc 1: south-bump (PI→0, traces south semicircle)
        mH7FloorArc1 = meshFactory.makeArcFloor(ri, ro, PI, 0.0f, 48);
        // Arc 2: north-bump (PI→2*PI, traces north semicircle)
        mH7FloorArc2 = meshFactory.makeArcFloor(ri, ro, PI, 2.0f * PI, 48);
        mH7WallA1In = meshFactory.makeArcWall(ri, 0.28f, 0.45f, PI, 0.0f, 48);
        mH7WallA1Out = meshFactory.makeArcWall(ro, 0.28f, 0.45f, PI, 0.0f, 48);
        mH7WallA2In = meshFactory.makeArcWall(ri, 0.28f, 0.45f, PI, 2.0f * PI, 48);
        mH7WallA2Out = meshFactory.makeArcWall(ro, 0.28f, 0.45f, PI, 2.0f * PI, 48);
    }
    mH8Floor = meshFactory.makeTriFloor({0, 0, -H8_HW}, {0, 0, H8_HW}, {H8_LEN, 0, 0});
    mSkybox = meshFactory.makeSkyboxMesh();

    double prev = glfwGetTime();

    while (!glfwWindowShouldClose(gWin))
    {
        double now = glfwGetTime();
        float dt = (float)(now - prev);
        prev = now;

        // Auto time advance
        timeOfDay = fmodf(timeOfDay + todSpeed * dt, 1.f);

        // Ball update
        ball.update(dt);

        // Hill physics for hole 2
        if (gCurrentHole == 2 && ball.getActive())
        {
            float dx = ball.getPos().x - HOLE2_X;
            float dz = ball.getPos().z - HOLE2_Z;
            float d2 = dx * dx + dz * dz;
            float hr2 = HILL_R * HILL_R;
            if (d2 < hr2)
            {
                float d = sqrtf(d2);
                float t2 = 1.f - d2 / hr2;                                       // 1 at centre, 0 at edge
                float domeH = 0.01f + HILL_H * sqrtf(t2);                        // sphere surface world-Y
                ball.setPos({ball.getPos().x, BALL_R + domeH, ball.getPos().z}); // ball sits on top
                // gravity slope: parabolic approximation, smooth and bounded
                if (d > 0.01f)
                {
                    float slopeMag = 9.8f * 2.f * HILL_H * d / hr2;
                    vec3 downhill = {dx / d, 0.f, dz / d};
                    ball.setVel(ball.getVel() + downhill * slopeMag * dt);
                    if (glm::length(ball.getVel()) > 0.01f)
                        ball.setMoving(true);
                }
            }
            else
            {
                ball.setPos({ball.getPos().y, BALL_R, ball.getPos().z});
            }
        }
        else
        {
            ball.setPos({ball.getPos().y, BALL_R, ball.getPos().z});
        }

        // Hole completion → advance to next hole
        if (ball.getActive() && !ball.getInHole() && ball.nearCup())
        {
            ball.setInHole(true);
            ball.setMoving(false);
            printf("Hole %d: %d stroke%s\n",
                   gCurrentHole, ball.getStrokes(), ball.getStrokes() == 1 ? "" : "s");
            if (gCurrentHole < 7)
            {
                gCurrentHole++;
                float nx, nz, nteeZ;
                if (gCurrentHole == 2)
                {
                    nx = HOLE2_X;
                    nz = HOLE2_Z;
                    nteeZ = nz + 5.5f;
                }
                else if (gCurrentHole == 3)
                {
                    nx = HOLE3_X;
                    nz = HOLE3_Z;
                    nteeZ = nz + H3_R * 1.9f - 0.5f;
                }
                else if (gCurrentHole == 4)
                {
                    nx = HOLE4_CX + (H4_RI + H4_RO) * 0.5f;
                    nz = HOLE4_CZ;
                    nteeZ = nz;
                }
                else if (gCurrentHole == 5)
                {
                    nx = HOLE5_CX;
                    nz = HOLE5_CZ;
                    nteeZ = HOLE5_CZ + H5_R - 1.5f;
                }
                else if (gCurrentHole == 6)
                {
                    nx = H6_CX;
                    nz = H6_TEE_Z;
                    nteeZ = H6_TEE_Z - 1.0f;
                }
                else
                {
                    nx = H7_X0 + H7_LS + 0.5f;
                    nz = H7_Z0;
                    nteeZ = H7_Z0;
                }
                ball.setPos({nx, BALL_R, nteeZ});
                ball.setVel({0, 0, 0});
                ball.setActive(true);
                ball.setMoving(false);
                ball.setInHole(false);
                ball.setStrokes(0);
                aimMode = false;
            }
            else
            {
                ball.setActive(false);
                printf("Course complete!\n");
            }
        }

        // Camera movement
        float spd = cam.getSpd() * dt;
        vec3 fw = cam.fwd();
        vec3 rt = glm::normalize(glm::cross(fw, {0, 1, 0}));
        if (glfwGetKey(gWin, GLFW_KEY_W) == GLFW_PRESS)
            cam.setPos(cam.getPos() + fw * spd);
        if (glfwGetKey(gWin, GLFW_KEY_S) == GLFW_PRESS)
            cam.setPos(cam.getPos() - fw * spd);
        if (glfwGetKey(gWin, GLFW_KEY_A) == GLFW_PRESS)
            cam.setPos(cam.getPos() - rt * spd);
        if (glfwGetKey(gWin, GLFW_KEY_D) == GLFW_PRESS)
            cam.setPos(cam.getPos() + rt * spd);
        if (glfwGetKey(gWin, GLFW_KEY_Q) == GLFW_PRESS)
            cam.setPos({cam.getPos().x, cam.getPos().y - spd, cam.getPos().z});
        if (glfwGetKey(gWin, GLFW_KEY_E) == GLFW_PRESS)
            cam.setPos({cam.getPos().x, cam.getPos().y + spd, cam.getPos().z});

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float asp = (float)WIN_W / WIN_H;
        mat4 view = cam.view();
        mat4 proj = cam.proj(asp);
        mat4 vp = proj * view;

        // Skybox
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        glUseProgram(skyProg);
        mat4 skyVP = proj * mat4(mat3(view));
        glUniformMatrix4fv(glGetUniformLocation(skyProg, "uVP"), 1, GL_FALSE, glm::value_ptr(skyVP));
        glUniform1f(glGetUniformLocation(skyProg, "uTimeOfDay"), timeOfDay);
        glBindVertexArray(mSkybox.VAO());
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        // Scene
        glUseProgram(gProg);
        setUniforms((float)now, vp);

        // Trapezoidal property grass
        draw(mTrap, mat4(1), vp, 2);

        // Holes
        drawHole(vp, HOLE1_X, HOLE1_Z);
        // drawHole2(vp, HOLE2_X, HOLE2_Z);
        // drawHole3(vp, HOLE3_X, HOLE3_Z);
        // drawHole4(vp);
        // drawHole5(vp);
        // drawHole6(vp);
        // drawHole7(vp);
        // drawHole8(vp);

        // Restaurant
        // drawRestaurant(vp);

        // Golf ball
        if (ball.getActive())
        {
            mat4 m = glm::translate(mat4(1), ball.getPos());
            m = glm::scale(m, {BALL_R * 2, BALL_R * 2, BALL_R * 2});
            draw(mSphere, m, vp, 15);
        }

        // Aim indicator — only while LMB is held
        if (aimMode && ball.getActive() &&
            glfwGetMouseButton(gWin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            vec3 diff = {aimTarget.x - ball.getPos().x, 0, aimTarget.z - ball.getPos().z};
            float len = glm::length(diff);
            if (len > 0.05f)
            {
                vec3 dir = diff / len;
                float ang = atan2f(dir.x, dir.z);
                // Raise line above ball (and above any hill) so it's always visible
                float lineY = ball.getPos().y + 0.12f;
                vec3 mid = {ball.getPos().x + dir.x * (len * 0.5f),
                            lineY,
                            ball.getPos().z + dir.z * (len * 0.5f)};
                mat4 m = glm::translate(mat4(1), mid);
                m = glm::rotate(m, ang, {0, 1, 0});
                m = glm::scale(m, {0.07f, 0.07f, len});
                draw(mBox, m, vp, 17);
            }
        }

        glfwSwapBuffers(gWin);
        glfwPollEvents();
    }

    mQuad.freeMesh();
    mBox.freeMesh();
    mSphere.freeMesh();
    mCylinder.freeMesh();
    mTorus.freeMesh();
    mTrap.freeMesh();
    mCircle.freeMesh();
    mH3Wall1.freeMesh();
    mH3Wall2.freeMesh();
    mH4Floor.freeMesh();
    mH4WallIn.freeMesh();
    mH4WallOut.freeMesh();
    mH5Floor.freeMesh();
    mH7FloorArc1.freeMesh();
    mH7FloorArc2.freeMesh();
    mH7WallA1In.freeMesh();
    mH7WallA1Out.freeMesh();
    mH7WallA2In.freeMesh();
    mH7WallA2Out.freeMesh();
    mSkybox.freeMesh();

    glDeleteProgram(gProg);
    glDeleteProgram(skyProg);
    glfwDestroyWindow(gWin);
    glfwTerminate();
    return 0;
}
