#include "Ball.h"

Ball::Ball()
    : pos({HOLE1_X, BALL_R, HOLE1_Z + 5.5f}), vel({0, 0, 0}), active(false), moving(false), inHole(false), strokes(0) {}

void Ball::update(float dt)
{
    if (!active || !moving)
        return;
    pos += vel * dt;
    vel *= powf(0.965f, dt * 60.f);
    if (glm::length(vel) < 0.02f)
    {
        vel = {0, 0, 0};
        moving = false;
    }
    wallCollide();
    checkObstacles();
}

void Ball::wallCollide()
{
    const float r = 0.72f;
    if (gCurrentHole <= 2)
    {
        float cx = (gCurrentHole == 1) ? HOLE1_X : HOLE2_X;
        float cz = (gCurrentHole == 1) ? HOLE1_Z : HOLE2_Z;
        if (pos.x - BALL_R < cx - FW_X)
        {
            pos.x = cx - FW_X + BALL_R;
            vel.x = fabsf(vel.x) * r;
        }
        if (pos.x + BALL_R > cx + FW_X)
        {
            pos.x = cx + FW_X - BALL_R;
            vel.x = -fabsf(vel.x) * r;
        }
        if (pos.z - BALL_R < cz + FW_ZN)
        {
            pos.z = cz + FW_ZN + BALL_R;
            vel.z = fabsf(vel.z) * r;
        }
        if (pos.z + BALL_R > cz + FW_ZS)
        {
            pos.z = cz + FW_ZS - BALL_R;
            vel.z = -fabsf(vel.z) * r;
        }
    }
    else if (gCurrentHole == 3)
    {
        // Hole 3 — circular figure-8 collision
        float lc1z = HOLE3_Z + H3_R * 0.9f;
        float lc2z = HOLE3_Z - H3_R * 0.9f;
        float dx = pos.x - HOLE3_X;
        float dz1 = pos.z - lc1z, dz2 = pos.z - lc2z;
        float d1 = sqrtf(dx * dx + dz1 * dz1);
        float d2 = sqrtf(dx * dx + dz2 * dz2);
        float maxR = H3_R - BALL_R;
        if (d1 > maxR && d2 > maxR)
        {
            float nx, nz;
            if (d1 <= d2)
            {
                nx = dx / d1;
                nz = dz1 / d1;
                pos.x = HOLE3_X + nx * maxR;
                pos.z = lc1z + nz * maxR;
            }
            else
            {
                nx = dx / d2;
                nz = dz2 / d2;
                pos.x = HOLE3_X + nx * maxR;
                pos.z = lc2z + nz * maxR;
            }
            float vdn = vel.x * nx + vel.z * nz;
            if (vdn > 0)
            {
                vel.x -= (1.f + r) * vdn * nx;
                vel.z -= (1.f + r) * vdn * nz;
            }
        }
    }
    else if (gCurrentHole == 4)
    {
        // Hole 4 — banana arc: radial walls + flat end caps
        float dx = pos.x - HOLE4_CX, dz = pos.z - HOLE4_CZ;
        float d = sqrtf(dx * dx + dz * dz);
        if (d < 0.01f)
            return;
        float nx = dx / d, nz = dz / d;
        // Outer curved wall
        if (d > H4_RO - BALL_R)
        {
            pos.x = HOLE4_CX + nx * (H4_RO - BALL_R);
            pos.z = HOLE4_CZ + nz * (H4_RO - BALL_R);
            float vdn = vel.x * nx + vel.z * nz;
            if (vdn > 0)
            {
                vel.x -= (1.f + r) * vdn * nx;
                vel.z -= (1.f + r) * vdn * nz;
            }
        }
        // Inner curved wall
        if (d < H4_RI + BALL_R)
        {
            pos.x = HOLE4_CX + nx * (H4_RI + BALL_R);
            pos.z = HOLE4_CZ + nz * (H4_RI + BALL_R);
            float vdn = vel.x * nx + vel.z * nz;
            if (vdn < 0)
            {
                vel.x -= (1.f + r) * vdn * nx;
                vel.z -= (1.f + r) * vdn * nz;
            }
        }
        // Tee end cap (east face): block ball going east past tee
        if (pos.x > HOLE4_CX + H4_RO - BALL_R)
        {
            pos.x = HOLE4_CX + H4_RO - BALL_R;
            if (vel.x > 0)
                vel.x = -fabsf(vel.x) * r;
        }
        // Tee end cap (south face): block ball going south past tee row
        if (pos.z > HOLE4_CZ + BALL_R)
        {
            pos.z = HOLE4_CZ + BALL_R;
            if (vel.z > 0)
                vel.z = -fabsf(vel.z) * r;
        }
        // Cup end cap: z floor  (sinf(-1.41)≈-0.9871)
        float cupEndZ = HOLE4_CZ + H4_RO * sinf(H4_T0) + BALL_R;
        if (pos.z < cupEndZ)
        {
            pos.z = cupEndZ;
            if (vel.z < 0)
                vel.z = fabsf(vel.z) * r;
        }
    }
    else if (gCurrentHole == 8)
    {
        // Hole 8 — triangle, apex west, base east
        const float bx = H8_BX, cz = H8_CZ, hw = H8_HW, len = H8_LEN;
        const float diagLen = sqrtf(len * len + hw * hw);
        // Base wall (east)
        if (pos.x > bx + len - BALL_R)
        {
            pos.x = bx + len - BALL_R;
            if (vel.x > 0)
                vel.x = -fabsf(vel.x) * r;
        }
        // Apex cap (west)
        if (pos.x < bx + BALL_R)
        {
            pos.x = bx + BALL_R;
            if (vel.x < 0)
                vel.x = fabsf(vel.x) * r;
        }
        // South wall: east-base-bottom→west-apex, inward normal = (hw,-len)/diagLen
        {
            float nx = hw / diagLen, nz = -len / diagLen;
            float d = (pos.x - (bx + len)) * nx + (pos.z - (cz + hw)) * nz;
            if (d < BALL_R)
            {
                float push = BALL_R - d;
                pos.x += push * nx;
                pos.z += push * nz;
                float vn = vel.x * nx + vel.z * nz;
                if (vn < 0)
                {
                    vel.x -= (1.f + r) * vn * nx;
                    vel.z -= (1.f + r) * vn * nz;
                }
            }
        }
        // North wall: west-apex→east-base-top, inward normal = (hw,len)/diagLen
        {
            float nx = hw / diagLen, nz = len / diagLen;
            float d = (pos.x - bx) * nx + (pos.z - cz) * nz;
            if (d < BALL_R)
            {
                float push = BALL_R - d;
                pos.x += push * nx;
                pos.z += push * nz;
                float vn = vel.x * nx + vel.z * nz;
                if (vn < 0)
                {
                    vel.x -= (1.f + r) * vn * nx;
                    vel.z -= (1.f + r) * vn * nz;
                }
            }
        }
    }
    else if (gCurrentHole == 7)
    {
        // Hole 7 — S-shape: two semicircle arcs, no straight sections
        const float ri = H7_RA - H7_FW * 0.5f;
        const float ro = H7_RA + H7_FW * 0.5f;
        const float c1x = H7_X0 + H7_LS + H7_RA;        // -8
        const float c2x = H7_X0 + H7_LS + 3.0f * H7_RA; //  2
        const float cz0 = H7_Z0;

        // Hard outer boundaries (outer arc wall extents)
        if (pos.x < c1x - ro + BALL_R)
        {
            pos.x = c1x - ro + BALL_R;
            if (vel.x < 0)
                vel.x = fabsf(vel.x) * r;
        }
        if (pos.x > c2x + ro - BALL_R)
        {
            pos.x = c2x + ro - BALL_R;
            if (vel.x > 0)
                vel.x = -fabsf(vel.x) * r;
        }

        // Pick nearest arc centre and apply radial collision
        float midX = (c1x + c2x) * 0.5f; // -3
        float cx = (pos.x <= midX) ? c1x : c2x;
        float dx = pos.x - cx, dz = pos.z - cz0;
        float d = sqrtf(dx * dx + dz * dz);
        if (d > 0.01f)
        {
            float nx = dx / d, nz = dz / d;
            if (d > ro - BALL_R)
            {
                pos.x = cx + nx * (ro - BALL_R);
                pos.z = cz0 + nz * (ro - BALL_R);
                float vdn = vel.x * nx + vel.z * nz;
                if (vdn > 0)
                {
                    vel.x -= (1.f + r) * vdn * nx;
                    vel.z -= (1.f + r) * vdn * nz;
                }
            }
            if (d < ri + BALL_R)
            {
                pos.x = cx + nx * (ri + BALL_R);
                pos.z = cz0 + nz * (ri + BALL_R);
                float vdn = vel.x * nx + vel.z * nz;
                if (vdn < 0)
                {
                    vel.x -= (1.f + r) * vdn * nx;
                    vel.z -= (1.f + r) * vdn * nz;
                }
            }
        }
    }
    else if (gCurrentHole == 6)
    {
        // Hole 6 — L-shape: axis-aligned walls for each face
        const float xW = H6_CX - H6_S1_W * 0.5f;
        const float xE1 = H6_CX + H6_S1_W * 0.5f;
        const float xE2 = xW + H6_S2_L;
        const float zS = H6_TEE_Z;
        const float zJ = H6_TEE_Z - H6_S1_L;
        const float zN = zJ - H6_S2_W;
        // West outer wall
        if (pos.x < xW + BALL_R)
        {
            pos.x = xW + BALL_R;
            if (vel.x < 0)
                vel.x = fabsf(vel.x) * r;
        }
        // South wall (tee end of S1)
        if (pos.z > zS - BALL_R)
        {
            pos.z = zS - BALL_R;
            if (vel.z > 0)
                vel.z = -fabsf(vel.z) * r;
        }
        // North wall (far end of S2)
        if (pos.z < zN + BALL_R)
        {
            pos.z = zN + BALL_R;
            if (vel.z < 0)
                vel.z = fabsf(vel.z) * r;
        }
        // East wall of S2 (only when in S2 z-range)
        if (pos.z <= zJ && pos.x > xE2 - BALL_R)
        {
            pos.x = xE2 - BALL_R;
            if (vel.x > 0)
                vel.x = -fabsf(vel.x) * r;
        }
        // East wall of S1 (inner right face, only when in S1 z-range)
        if (pos.z > zJ && pos.x > xE1 - BALL_R)
        {
            pos.x = xE1 - BALL_R;
            if (vel.x > 0)
                vel.x = -fabsf(vel.x) * r;
        }
        // Inner south wall of S2 (only east of inner corner)
        if (pos.x > xE1 && pos.z > zJ - BALL_R)
        {
            pos.z = zJ - BALL_R;
            if (vel.z > 0)
                vel.z = -fabsf(vel.z) * r;
        }
    }
    else if (gCurrentHole == 5)
    {
        // Hole 5 — convex pentagon half-plane collision
        for (int k = 0; k < 5; k++)
        {
            float t0 = PI / 2.0f + k * (2.0f * PI / 5.0f), t1 = PI / 2.0f + (k + 1) * (2.0f * PI / 5.0f);
            float ax = HOLE5_CX + H5_R * cosf(t0), az = HOLE5_CZ + H5_R * sinf(t0);
            float bx = HOLE5_CX + H5_R * cosf(t1), bz = HOLE5_CZ + H5_R * sinf(t1);
            float edx = bx - ax, edz = bz - az;
            float len = sqrtf(edx * edx + edz * edz);
            float onx = edz / len, onz = -edx / len; // outward normal
            float d = (pos.x - ax) * onx + (pos.z - az) * onz;
            if (d > -BALL_R)
            {
                float pen = d + BALL_R;
                pos.x -= onx * pen;
                pos.z -= onz * pen;
                float vdn = vel.x * onx + vel.z * onz;
                if (vdn > 0)
                {
                    vel.x -= (1.f + r) * vdn * onx;
                    vel.z -= (1.f + r) * vdn * onz;
                }
            }
        }
    }
}

void Ball::checkObstacles()
{
    if (gCurrentHole != 3)
        return;
    const float obsR = H3_OBSR, r = 0.72f;
    float lc1z = HOLE3_Z + H3_R * 0.9f;
    float lc2z = HOLE3_Z - H3_R * 0.9f;
    float obsZs[2] = {lc1z, lc2z};
    for (int i = 0; i < 2; i++)
    {
        float dx = pos.x - HOLE3_X, dz = pos.z - obsZs[i];
        float d2 = dx * dx + dz * dz, minD = BALL_R + obsR;
        if (d2 < minD * minD && d2 > 0.0001f)
        {
            float d = sqrtf(d2), push = minD - d;
            float nx = dx / d, nz = dz / d;
            pos.x += nx * push;
            pos.z += nz * push;
            float vdn = vel.x * nx + vel.z * nz;
            if (vdn < 0)
            {
                vel.x -= (1.f + r) * vdn * nx;
                vel.z -= (1.f + r) * vdn * nz;
            }
        }
    }
}

bool Ball::nearCup() {
    if(gCurrentHole==1)
            return glm::length(vec3(pos.x-HOLE1_X,0,pos.z-(HOLE1_Z-5.5f))) < 0.35f;
        if(gCurrentHole==2)
            return glm::length(vec3(pos.x-HOLE2_X,0,pos.z-(HOLE2_Z-5.5f))) < 0.35f;
        if(gCurrentHole==3){
            float cupZ = HOLE3_Z - H3_R*0.9f - H3_R + 0.5f;
            return glm::length(vec3(pos.x-HOLE3_X,0,pos.z-cupZ)) < 0.35f;
        }
        if(gCurrentHole==4){
            float rmid=(H4_RI+H4_RO)*0.5f;
            float cx4=HOLE4_CX+rmid*cosf(H4_T0), cz4=HOLE4_CZ+rmid*sinf(H4_T0);
            return glm::length(vec3(pos.x-cx4,0,pos.z-cz4)) < 0.35f;
        }
        if(gCurrentHole==5){
            float t2=PI/2.0f+2.0f*(2.0f*PI/5.0f), t3=PI/2.0f+3.0f*(2.0f*PI/5.0f);
            float cupX=HOLE5_CX+H5_R*(cosf(t2)+cosf(t3))*0.5f;
            float cupZ=HOLE5_CZ+H5_R*(sinf(t2)+sinf(t3))*0.5f+1.2f;
            return glm::length(vec3(pos.x-cupX,0,pos.z-cupZ)) < 0.35f;
        }
        if(gCurrentHole==6){
            float cupX = H6_CX - H6_S1_W*0.5f + H6_S2_L - 0.5f;
            float cupZ = H6_TEE_Z - H6_S1_L - H6_S2_W*0.5f;
            return glm::length(vec3(pos.x-cupX,0,pos.z-cupZ)) < 0.35f;
        }
        if(gCurrentHole==7){
            float cx9 = H7_X0 + H7_LS + 4.0f*H7_RA - 1.0f;
            float cz9 = H7_Z0 - 1.0f;
            return glm::length(vec3(pos.x-cx9,0,pos.z-cz9)) < 0.35f;
        }
        if(gCurrentHole==8){
            float cx = H8_BX + H8_LEN - 1.5f;
            return glm::length(vec3(pos.x-cx,0,pos.z-H8_CZ)) < 0.35f;
        }
        return false;
}

void Ball::setPos(vec3 p) {
    this->pos = p;
}

void Ball::setVel(vec3 v) {
    this->vel = v;
}

void Ball::setActive(bool a) {
    this->active = a;
}

void Ball::setMoving(bool m) {
    this->moving = m;
}

void Ball::setInHole(bool ih) {
    this->inHole = ih;
}

void Ball::setStrokes(int s) {
    this->strokes = s;
}

vec3 Ball::getPos() const {
    return pos;
}

vec3 Ball::getVel() const {
    return vel;
}

bool Ball::getActive() const {
    return active;
}

bool Ball::getMoving() const {
    return moving;
}

bool Ball::getInHole() const {
    return inHole;
}

int Ball::getStrokes() const {
    return strokes;
}
