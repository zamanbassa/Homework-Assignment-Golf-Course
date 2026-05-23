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

using namespace std;
using namespace glm;

// ─── Camera ──────────────────────────────────────────────────────────────────

MeshFactory meshFactory;

// ─── Globals ──────────────────────────────────────────────────────────────────
static GLuint gProg, skyProg;
static Mesh mQuad, mBox, mSphere, mCylinder, mTorus, mTrap, mSkybox, mCircle;
static Mesh mH3Wall1, mH3Wall2;                                   // hole 3 arc walls (south loop, north loop)
static Mesh mH4Floor, mH4WallIn, mH4WallOut;                      // hole 4 banana meshes
static Mesh mH5Floor;                                             // hole 5 pentagon floor
static Mesh mH7FloorArc1, mH7FloorArc2;                           // hole 7 S-shape floors
static Mesh mH7WallA1In, mH7WallA1Out, mH7WallA2In, mH7WallA2Out; // hole 7 S-shape walls
static Mesh mH8Floor;                                             // hole 8 triangle floor
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

// ─── draw() shorthand ────────────────────────────────────────────────────────
static void draw(const Mesh &m, const mat4 &model, const mat4 &vp, int surf)
{
    mat4 mvp = vp * model;
    glUniformMatrix4fv(glGetUniformLocation(gProg, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(glGetUniformLocation(gProg, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(glGetUniformLocation(gProg, "uSurface"), surf);
    glBindVertexArray(m.VAO());
    glDrawElements(GL_TRIANGLES, m.Count(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

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

// ─── Draw a golf hole centred at (cx, 0, cz) ─────────────────────────────────
static void drawHole(const mat4 &vp, float cx, float cz)
{
    const float FW = 4.0f;
    const float FL = 14.0f;
    const float WH = 0.45f;
    const float WTH = 0.30f;

    // Fairway
    {
        mat4 m = glm::translate(mat4(1), {cx, 0, cz});
        m = glm::scale(m, {FW, 1, FL});
        draw(mQuad, m, vp, 0);
    }
    // Tee box (south end)
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.005f, cz + 5.5f});
        m = glm::scale(m, {FW - 0.2f, 1, 1.5f});
        draw(mQuad, m, vp, 1);
    }
    // Water hazard
    {
        mat4 m = glm::translate(mat4(1), {cx, -0.01f, cz});
        m = glm::scale(m, {FW, 1, 2.2f});
        draw(mQuad, m, vp, 5);
    }
    // Bridge + railings
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.06f, cz});
        m = glm::scale(m, {1.9f, 0.14f, 2.5f});
        draw(mBox, m, vp, 6);
        for (int s = -1; s <= 1; s += 2)
        {
            mat4 r = glm::translate(mat4(1), {cx + (float)s * 0.85f, 0.26f, cz});
            r = glm::scale(r, {0.09f, 0.45f, 2.5f});
            draw(mBox, r, vp, 6);
        }
    }
    // Side walls
    for (int s = -1; s <= 1; s += 2)
    {
        mat4 m = glm::translate(mat4(1), {cx + (FW * 0.5f + WTH * 0.5f) * (float)s, WH * 0.5f, cz});
        m = glm::scale(m, {WTH, WH, FL + WTH * 2});
        draw(mBox, m, vp, 8);
    }
    // North end wall
    {
        mat4 m = glm::translate(mat4(1), {cx, WH * 0.5f, cz - FL * 0.5f - WTH * 0.5f});
        m = glm::scale(m, {FW + WTH * 2, WH, WTH});
        draw(mBox, m, vp, 8);
    }
    // South end wall
    {
        mat4 m = glm::translate(mat4(1), {cx, WH * 0.5f, cz + FL * 0.5f + WTH * 0.5f});
        m = glm::scale(m, {FW + WTH * 2, WH, WTH});
        draw(mBox, m, vp, 8);
    }
    // Cup torus + disc (north end = cup)
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.01f, cz - 5.5f});
        m = glm::scale(m, {0.35f, 0.1f, 0.35f});
        draw(mTorus, m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.001f, cz - 5.5f});
        m = glm::scale(m, {0.33f, 1, 0.33f});
        draw(mQuad, m, vp, 19);
    }
    // Flag pole
    {
        mat4 m = glm::translate(mat4(1), {cx + 0.38f, 0, cz - 5.5f});
        m = glm::scale(m, {0.05f, 2.2f, 0.05f});
        draw(mCylinder, m, vp, 16);
    }
    // Flag pennant — vertical
    {
        mat4 m = glm::translate(mat4(1), {cx + 0.655f, 2.0f, cz - 5.5f});
        m = glm::rotate(m, PI * 0.5f, {1, 0, 0});
        m = glm::scale(m, {0.55f, 1.0f, 0.32f});
        draw(mQuad, m, vp, 17);
    }
}

// ─── Hole 2 — same layout, hill instead of water+bridge ──────────────────────
static void drawHole2(const mat4 &vp, float cx, float cz)
{
    const float FW = 4.0f;
    const float FL = 14.0f;
    const float WH = 0.45f;
    const float WTH = 0.30f;

    // Hill drawn first so the fairway quad covers its underground half
    // Scale = HILL_R (not *2) because mSphere is radius-1, so scale==world-radius
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.01f, cz});
        m = glm::scale(m, {HILL_R, HILL_H, HILL_R});
        draw(mSphere, m, vp, 0);
    }
    // Fairway (covers underground hemisphere, drawn after hill)
    {
        mat4 m = glm::translate(mat4(1), {cx, 0, cz});
        m = glm::scale(m, {FW, 1, FL});
        draw(mQuad, m, vp, 0);
    }
    // Tee box
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.005f, cz + 5.5f});
        m = glm::scale(m, {FW - 0.2f, 1, 1.5f});
        draw(mQuad, m, vp, 1);
    }
    // Side walls
    for (int s = -1; s <= 1; s += 2)
    {
        mat4 m = glm::translate(mat4(1), {cx + (FW * 0.5f + WTH * 0.5f) * (float)s, WH * 0.5f, cz});
        m = glm::scale(m, {WTH, WH, FL + WTH * 2});
        draw(mBox, m, vp, 8);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx, WH * 0.5f, cz - FL * 0.5f - WTH * 0.5f});
        m = glm::scale(m, {FW + WTH * 2, WH, WTH});
        draw(mBox, m, vp, 8);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx, WH * 0.5f, cz + FL * 0.5f + WTH * 0.5f});
        m = glm::scale(m, {FW + WTH * 2, WH, WTH});
        draw(mBox, m, vp, 8);
    }
    // Cup
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.01f, cz - 5.5f});
        m = glm::scale(m, {0.35f, 0.1f, 0.35f});
        draw(mTorus, m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.001f, cz - 5.5f});
        m = glm::scale(m, {0.33f, 1, 0.33f});
        draw(mQuad, m, vp, 19);
    }
    // Flag pole
    {
        mat4 m = glm::translate(mat4(1), {cx + 0.38f, 0, cz - 5.5f});
        m = glm::scale(m, {0.05f, 2.2f, 0.05f});
        draw(mCylinder, m, vp, 16);
    }
    // Flag pennant
    {
        mat4 m = glm::translate(mat4(1), {cx + 0.655f, 2.0f, cz - 5.5f});
        m = glm::rotate(m, PI * 0.5f, {1, 0, 0});
        m = glm::scale(m, {0.55f, 1.0f, 0.32f});
        draw(mQuad, m, vp, 17);
    }
}

// ─── Hole 3 — true circular figure-8 ────────────────────────────────────────
static void drawHole3(const mat4 &vp, float cx, float cz)
{
    float lc1z = cz + H3_R * 0.9f; // south loop centre
    float lc2z = cz - H3_R * 0.9f; // north loop centre
    float cupZ = lc2z - H3_R + 0.5f;
    float teeZ = lc1z + H3_R - 0.5f;

    // Circular floors
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.0f, lc1z});
        m = glm::scale(m, {H3_R, 1, H3_R});
        draw(mCircle, m, vp, 0);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.0f, lc2z});
        m = glm::scale(m, {H3_R, 1, H3_R});
        draw(mCircle, m, vp, 0);
    }

    // Tee box
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.005f, teeZ});
        m = glm::scale(m, {2.0f, 1, 1.0f});
        draw(mQuad, m, vp, 1);
    }

    // Seamless arc walls (pre-built meshes, translated to each loop centre)
    {
        mat4 m = glm::translate(mat4(1), {cx, 0, lc1z});
        draw(mH3Wall1, m, vp, 8);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx, 0, lc2z});
        draw(mH3Wall2, m, vp, 8);
    }

    // Rock obstacles at loop centres
    {
        mat4 m = glm::translate(mat4(1), {cx, 0, lc1z});
        m = glm::scale(m, {H3_OBSR * 2, 0.40f, H3_OBSR * 2});
        draw(mCylinder, m, vp, 10);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx, 0, lc2z});
        m = glm::scale(m, {H3_OBSR * 2, 0.40f, H3_OBSR * 2});
        draw(mCylinder, m, vp, 10);
    }

    // Cup
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.01f, cupZ});
        m = glm::scale(m, {0.35f, 0.1f, 0.35f});
        draw(mTorus, m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.001f, cupZ});
        m = glm::scale(m, {0.33f, 1, 0.33f});
        draw(mQuad, m, vp, 19);
    }
    // Flag pole + pennant
    {
        mat4 m = glm::translate(mat4(1), {cx + 0.38f, 0, cupZ});
        m = glm::scale(m, {0.05f, 2.2f, 0.05f});
        draw(mCylinder, m, vp, 16);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx + 0.655f, 2.0f, cupZ});
        m = glm::rotate(m, PI * 0.5f, {1, 0, 0});
        m = glm::scale(m, {0.55f, 1.0f, 0.32f});
        draw(mQuad, m, vp, 17);
    }
}

// ─── Hole 4 — banana arc curving left ────────────────────────────────────────
static void drawHole4(const mat4 &vp)
{
    const float cx = HOLE4_CX, cz = HOLE4_CZ;
    const float WH = 0.45f, WTH = 0.28f;
    const float RMID = (H4_RI + H4_RO) * 0.5f; // 9.0

    // Banana floor
    {
        mat4 m = glm::translate(mat4(1), {cx, 0, cz});
        draw(mH4Floor, m, vp, 0);
    }

    // Tee box (at east end, t=0)
    {
        mat4 m = glm::translate(mat4(1), {cx + RMID, 0.005f, cz});
        m = glm::scale(m, {H4_RO - H4_RI, 1, 1.5f});
        draw(mQuad, m, vp, 1);
    }

    // Curved walls
    {
        mat4 m = glm::translate(mat4(1), {cx, 0, cz});
        draw(mH4WallIn, m, vp, 8);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx, 0, cz});
        draw(mH4WallOut, m, vp, 8);
    }

    // Cup end cap (northwest) — rotate by -H4_T0 so local +x aligns with radial direction
    {
        float ct = cosf(H4_T0), st = sinf(H4_T0);
        float ecx = cx + RMID * ct, ecz = cz + RMID * st;
        mat4 m = glm::translate(mat4(1), {ecx, WH * 0.5f, ecz});
        m = glm::rotate(m, -H4_T0, {0, 1, 0});
        m = glm::scale(m, {H4_RO - H4_RI + WTH * 2, WH, WTH});
        draw(mBox, m, vp, 8);
    }

    // Cup torus + disc
    {
        float cx4 = cx + RMID * cosf(H4_T0), cz4 = cz + RMID * sinf(H4_T0);
        {
            mat4 m = glm::translate(mat4(1), {cx4, 0.01f, cz4});
            m = glm::scale(m, {0.35f, 0.1f, 0.35f});
            draw(mTorus, m, vp, 19);
        }
        {
            mat4 m = glm::translate(mat4(1), {cx4, 0.001f, cz4});
            m = glm::scale(m, {0.33f, 1, 0.33f});
            draw(mQuad, m, vp, 19);
        }
        {
            mat4 m = glm::translate(mat4(1), {cx4 + 0.38f, 0, cz4});
            m = glm::scale(m, {0.05f, 2.2f, 0.05f});
            draw(mCylinder, m, vp, 16);
        }
        {
            mat4 m = glm::translate(mat4(1), {cx4 + 0.655f, 2.0f, cz4});
            m = glm::rotate(m, PI * 0.5f, {1, 0, 0});
            m = glm::scale(m, {0.55f, 1.0f, 0.32f});
            draw(mQuad, m, vp, 17);
        }
    }
}

// ─── Hole 5 — regular pentagon, tee at south vertex, cup at north edge mid ───
static void drawHole5(const mat4 &vp)
{
    const float cx = HOLE5_CX, cz = HOLE5_CZ;
    const float WH = 0.45f, WTH = 0.30f;

    // Pentagon floor (scale = circumradius)
    {
        mat4 m = glm::translate(mat4(1), {cx, 0, cz});
        m = glm::scale(m, {H5_R, 1, H5_R});
        draw(mH5Floor, m, vp, 0);
    }

    // Tee box near south vertex (inset 1.8 from vertex so it stays inside walls)
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.005f, cz + H5_R - 1.8f});
        m = glm::scale(m, {2.0f, 1, 1.0f});
        draw(mQuad, m, vp, 1);
    }

    // 5 walls, one per edge
    float edgeLen = 2.0f * H5_R * sinf(PI / 5.0f);
    for (int k = 0; k < 5; k++)
    {
        float t0 = PI / 2.0f + k * (2.0f * PI / 5.0f), t1 = PI / 2.0f + (k + 1) * (2.0f * PI / 5.0f);
        float x0 = H5_R * cosf(t0), z0 = H5_R * sinf(t0);
        float x1 = H5_R * cosf(t1), z1 = H5_R * sinf(t1);
        float mx = (x0 + x1) * 0.5f, mz = (z0 + z1) * 0.5f;
        float edx = x1 - x0, edz = z1 - z0;
        float ang = atan2f(-edz, edx); // GLM Y-rot: +x → (cosθ,0,-sinθ)
        mat4 m = glm::translate(mat4(1), {cx + mx, WH * 0.5f, cz + mz});
        m = glm::rotate(m, ang, {0, 1, 0});
        m = glm::scale(m, {edgeLen + WTH, WH, WTH});
        draw(mBox, m, vp, 8);
    }

    // Cup at midpoint of north edge (V2→V3), inset 1.2 from wall
    float t2 = PI / 2.0f + 2.0f * (2.0f * PI / 5.0f), t3 = PI / 2.0f + 3.0f * (2.0f * PI / 5.0f);
    float cupX = cx + H5_R * (cosf(t2) + cosf(t3)) * 0.5f;
    float cupZ = cz + H5_R * (sinf(t2) + sinf(t3)) * 0.5f + 1.2f;
    {
        mat4 m = glm::translate(mat4(1), {cupX, 0.01f, cupZ});
        m = glm::scale(m, {0.35f, 0.1f, 0.35f});
        draw(mTorus, m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cupX, 0.001f, cupZ});
        m = glm::scale(m, {0.33f, 1, 0.33f});
        draw(mQuad, m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cupX + 0.38f, 0, cupZ});
        m = glm::scale(m, {0.05f, 2.2f, 0.05f});
        draw(mCylinder, m, vp, 16);
    }
    {
        mat4 m = glm::translate(mat4(1), {cupX + 0.655f, 2.0f, cupZ});
        m = glm::rotate(m, PI * 0.5f, {1, 0, 0});
        m = glm::scale(m, {0.55f, 1.0f, 0.32f});
        draw(mQuad, m, vp, 17);
    }
}

// ─── Hole 6 — L-shape (short N-S arm + long E-W arm at top of course) ───────
static void drawHole6(const mat4 &vp)
{
    const float WH = 0.45f, WTH = 0.30f;
    const float xW = H6_CX - H6_S1_W * 0.5f;  // -35  west boundary
    const float xE1 = H6_CX + H6_S1_W * 0.5f; // -31  inner corner x
    const float xE2 = xW + H6_S2_L;           // -19  cup end
    const float zS = H6_TEE_Z;                // -38  south / tee end
    const float zJ = H6_TEE_Z - H6_S1_L;      // -44.5 junction (S1 north = S2 south)
    const float zN = zJ - H6_S2_W;            // -48.5 north end

    // S1 fairway (short arm, N-S)
    {
        mat4 m = glm::translate(mat4(1), {H6_CX, 0.0f, (zS + zJ) * 0.5f});
        m = glm::scale(m, {H6_S1_W, 1, H6_S1_L});
        draw(mQuad, m, vp, 0);
    }
    // S2 fairway (long arm, E-W)
    {
        mat4 m = glm::translate(mat4(1), {(xW + xE2) * 0.5f, 0.0f, (zJ + zN) * 0.5f});
        m = glm::scale(m, {H6_S2_L, 1, H6_S2_W});
        draw(mQuad, m, vp, 0);
    }

    // Tee box (inset 1 unit from south wall)
    {
        mat4 m = glm::translate(mat4(1), {H6_CX, 0.005f, zS - 1.0f});
        m = glm::scale(m, {H6_S1_W - 0.2f, 1, 1.2f});
        draw(mQuad, m, vp, 1);
    }

    // Wall 1: south wall of S1
    {
        mat4 m = glm::translate(mat4(1), {H6_CX, WH * 0.5f, zS + WTH * 0.5f});
        m = glm::scale(m, {H6_S1_W + WTH * 2, WH, WTH});
        draw(mBox, m, vp, 8);
    }
    // Wall 2: west outer wall (full height of both arms combined)
    {
        float cz = (zS + zN) * 0.5f, len = fabsf(zN - zS);
        mat4 m = glm::translate(mat4(1), {xW - WTH * 0.5f, WH * 0.5f, cz});
        m = glm::scale(m, {WTH, WH, len + WTH * 2});
        draw(mBox, m, vp, 8);
    }
    // Wall 3: north wall of S2
    {
        mat4 m = glm::translate(mat4(1), {(xW + xE2) * 0.5f, WH * 0.5f, zN - WTH * 0.5f});
        m = glm::scale(m, {H6_S2_L + WTH * 2, WH, WTH});
        draw(mBox, m, vp, 8);
    }
    // Wall 4: east wall of S2 (cup end)
    {
        mat4 m = glm::translate(mat4(1), {xE2 + WTH * 0.5f, WH * 0.5f, (zJ + zN) * 0.5f});
        m = glm::scale(m, {WTH, WH, H6_S2_W + WTH * 2});
        draw(mBox, m, vp, 8);
    }
    // Wall 5: inner south wall of S2 (east of inner corner)
    {
        mat4 m = glm::translate(mat4(1), {(xE1 + xE2) * 0.5f, WH * 0.5f, zJ + WTH * 0.5f});
        m = glm::scale(m, {xE2 - xE1 + WTH * 2, WH, WTH});
        draw(mBox, m, vp, 8);
    }
    // Wall 6: east wall of S1 (inner right face, from tee to junction)
    {
        mat4 m = glm::translate(mat4(1), {xE1 + WTH * 0.5f, WH * 0.5f, (zS + zJ) * 0.5f});
        m = glm::scale(m, {WTH, WH, H6_S1_L + WTH * 2});
        draw(mBox, m, vp, 8);
    }

    // Cup at east end of S2, inset 0.5 from east wall
    const float cupX = xE2 - 0.5f, cupZ = (zJ + zN) * 0.5f;
    {
        mat4 m = glm::translate(mat4(1), {cupX, 0.01f, cupZ});
        m = glm::scale(m, {0.35f, 0.1f, 0.35f});
        draw(mTorus, m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cupX, 0.001f, cupZ});
        m = glm::scale(m, {0.33f, 1, 0.33f});
        draw(mQuad, m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cupX + 0.38f, 0, cupZ});
        m = glm::scale(m, {0.05f, 2.2f, 0.05f});
        draw(mCylinder, m, vp, 16);
    }
    {
        mat4 m = glm::translate(mat4(1), {cupX + 0.655f, 2.0f, cupZ});
        m = glm::rotate(m, PI * 0.5f, {1, 0, 0});
        m = glm::scale(m, {0.55f, 1.0f, 0.32f});
        draw(mQuad, m, vp, 17);
    }
}

// ─── Hole 7 — S-shape: two connected semicircle arcs (travels east) ──────────
static void drawHole7(const mat4 &vp)
{
    const float WH = 0.45f, WTH = 0.30f;
    const float ri = H7_RA - H7_FW * 0.5f;          // 3.0
    const float ro = H7_RA + H7_FW * 0.5f;          // 7.0
    const float c1x = H7_X0 + H7_LS + H7_RA;        // -8  (south-bump centre)
    const float c2x = H7_X0 + H7_LS + 3.0f * H7_RA; //  2  (north-bump centre)
    const float cz = H7_Z0;
    // Arc opening centres (where the S-ends are)
    const float teeX = c1x - H7_RA; // -13  centre of west (tee) opening
    const float cupX = c2x + H7_RA; //   7  centre of east (cup) opening

    // Arc floors
    {
        mat4 m = glm::translate(mat4(1), {c1x, 0, cz});
        draw(mH7FloorArc1, m, vp, 0);
    }
    {
        mat4 m = glm::translate(mat4(1), {c2x, 0, cz});
        draw(mH7FloorArc2, m, vp, 0);
    }

    // Tee mat
    {
        mat4 m = glm::translate(mat4(1), {teeX + 1.0f, 0.005f, cz + 1.0f});
        m = glm::scale(m, {ro - ri - 0.3f, 1, 1.5f});
        draw(mQuad, m, vp, 1);
    }

    // Arc curved walls
    {
        mat4 m = glm::translate(mat4(1), {c1x, 0, cz});
        draw(mH7WallA1In, m, vp, 8);
    }
    {
        mat4 m = glm::translate(mat4(1), {c1x, 0, cz});
        draw(mH7WallA1Out, m, vp, 8);
    }
    {
        mat4 m = glm::translate(mat4(1), {c2x, 0, cz});
        draw(mH7WallA2In, m, vp, 8);
    }
    {
        mat4 m = glm::translate(mat4(1), {c2x, 0, cz});
        draw(mH7WallA2Out, m, vp, 8);
    }

    // West end cap — bridges inner→outer wall ends at the tee opening
    {
        mat4 m = glm::translate(mat4(1), {teeX, WH * 0.5f, cz});
        m = glm::scale(m, {ro - ri + WTH, WH, WTH});
        draw(mBox, m, vp, 8);
    }
    // East end cap — bridges inner→outer wall ends at the cup opening
    {
        mat4 m = glm::translate(mat4(1), {cupX, WH * 0.5f, cz});
        m = glm::scale(m, {ro - ri + WTH, WH, WTH});
        draw(mBox, m, vp, 8);
    }

    // Cup — inward + shifted north into the arc belly
    const float cx9 = cupX - 1.0f;
    const float cz9 = cz - 1.0f;
    {
        mat4 m = glm::translate(mat4(1), {cx9, 0.01f, cz9});
        m = glm::scale(m, {0.35f, 0.1f, 0.35f});
        draw(mTorus, m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx9, 0.001f, cz9});
        m = glm::scale(m, {0.33f, 1, 0.33f});
        draw(mQuad, m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx9 + 0.38f, 0, cz9});
        m = glm::scale(m, {0.05f, 2.2f, 0.05f});
        draw(mCylinder, m, vp, 16);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx9 + 0.655f, 2.0f, cz9});
        m = glm::rotate(m, PI * 0.5f, {1, 0, 0});
        m = glm::scale(m, {0.55f, 1.0f, 0.32f});
        draw(mQuad, m, vp, 17);
    }
}

// ─── Hole 8 — triangle ────────────────────────────────────────────────────────
static void drawHole8(const mat4 &vp)
{
    const float WH = 0.45f, WTH = 0.28f;
    const float bx = H8_BX, cz = H8_CZ, hw = H8_HW, len = H8_LEN;
    const float diagLen = sqrtf(len * len + hw * hw);

    // Rotate entire hole 180° around its centre
    const float pivX = bx + len * 0.5f;
    mat4 R = glm::translate(mat4(1), {pivX, 0, cz}) * glm::rotate(mat4(1), PI, {0, 1, 0}) * glm::translate(mat4(1), {-pivX, 0, -cz});

    {
        mat4 m = glm::translate(mat4(1), {bx + len, 0, cz});
        m = glm::rotate(m, PI, {0, 1, 0});
        draw(mH8Floor, R * m, vp, 0);
    }

    {
        mat4 m = glm::translate(mat4(1), {bx + len - 0.8f, 0.005f, cz});
        m = glm::scale(m, {1.5f, 1, hw * 1.0f});
        draw(mQuad, R * m, vp, 1);
    }

    {
        mat4 m = glm::translate(mat4(1), {bx + len, WH * 0.5f, cz});
        m = glm::scale(m, {WTH, WH, 2 * hw + WTH});
        draw(mBox, R * m, vp, 8);
    }
    {
        mat4 m = glm::translate(mat4(1), {bx + len * 0.5f, WH * 0.5f, cz + hw * 0.5f});
        m = glm::rotate(m, atan2f(-hw, len), {0, 1, 0});
        m = glm::scale(m, {diagLen + WTH, WH, WTH});
        draw(mBox, R * m, vp, 8);
    }
    {
        mat4 m = glm::translate(mat4(1), {bx + len * 0.5f, WH * 0.5f, cz - hw * 0.5f});
        m = glm::rotate(m, atan2f(-hw, -len), {0, 1, 0});
        m = glm::scale(m, {diagLen + WTH, WH, WTH});
        draw(mBox, R * m, vp, 8);
    }

    const float cupX = bx + 1.5f;
    {
        mat4 m = glm::translate(mat4(1), {cupX, 0.01f, cz});
        m = glm::scale(m, {0.35f, 0.1f, 0.35f});
        draw(mTorus, R * m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cupX, 0.001f, cz});
        m = glm::scale(m, {0.33f, 1, 0.33f});
        draw(mQuad, R * m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cupX + 0.38f, 0, cz});
        m = glm::scale(m, {0.05f, 2.2f, 0.05f});
        draw(mCylinder, R * m, vp, 16);
    }
    {
        mat4 m = glm::translate(mat4(1), {cupX + 0.655f, 2.0f, cz});
        m = glm::rotate(m, PI * 0.5f, {1, 0, 0});
        m = glm::scale(m, {0.55f, 1.0f, 0.32f});
        draw(mQuad, R * m, vp, 17);
    }
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

// ─── Skybox ───────────────────────────────────────────────────────────────────

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
        drawHole2(vp, HOLE2_X, HOLE2_Z);
        drawHole3(vp, HOLE3_X, HOLE3_Z);
        drawHole4(vp);
        drawHole5(vp);
        drawHole6(vp);
        drawHole7(vp);
        drawHole8(vp);

        // Restaurant
        drawRestaurant(vp);

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
