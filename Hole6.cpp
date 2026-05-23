#include "Hole6.h"
#include <cmath>

Hole6::Hole6(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(6, q, b, c, t) {}

void Hole6::render(const mat4& vp) const {
    const float WH=H6_WH6, WTH=H6_WTH6;
    const float xW  = H6_CX - H6_S1_W*0.5f;       // -35  west boundary
    const float xE1 = H6_CX + H6_S1_W*0.5f;       // -31  inner corner x
    const float xE2 = xW + H6_S2_L;               // -19  cup end x
    const float zS  = H6_TEE_Z;                   // -38  south/tee end
    const float zJ  = H6_TEE_Z - H6_S1_L;        // -44.5 junction
    const float zN  = zJ - H6_S2_W;              // -48.5 north end

    // S1 fairway (short N-S arm)
    { mat4 m=glm::translate(mat4(1),{H6_CX,0,(zS+zJ)*0.5f}); m=glm::scale(m,{H6_S1_W,1,H6_S1_L}); draw(*mQuad,m,vp,0); }
    // S2 fairway (long E-W arm)
    { mat4 m=glm::translate(mat4(1),{(xW+xE2)*0.5f,0,(zJ+zN)*0.5f}); m=glm::scale(m,{H6_S2_L,1,H6_S2_W}); draw(*mQuad,m,vp,0); }
    // Tee box
    { mat4 m=glm::translate(mat4(1),{H6_CX,0.005f,zS-1.0f}); m=glm::scale(m,{H6_S1_W-0.2f,1,1.2f}); draw(*mQuad,m,vp,1); }
    // Walls
    drawWall(vp, H6_CX,               WH*0.5f, zS+WTH*0.5f,       H6_S1_W+WTH*2,          WH, WTH);
    drawWall(vp, xW-WTH*0.5f,         WH*0.5f, (zS+zN)*0.5f,      WTH, WH, fabsf(zN-zS)+WTH*2);
    drawWall(vp, (xW+xE2)*0.5f,       WH*0.5f, zN-WTH*0.5f,       H6_S2_L+WTH*2,          WH, WTH);
    drawWall(vp, xE2+WTH*0.5f,        WH*0.5f, (zJ+zN)*0.5f,      WTH, WH, H6_S2_W+WTH*2);
    drawWall(vp, (xE1+xE2)*0.5f,      WH*0.5f, zJ+WTH*0.5f,       xE2-xE1+WTH*2,          WH, WTH);
    drawWall(vp, xE1+WTH*0.5f,        WH*0.5f, (zS+zJ)*0.5f,      WTH, WH, H6_S1_L+WTH*2);
    // Cup
    drawCup(vp, xE2-0.5f, (zJ+zN)*0.5f);
}

void Hole6::wallCollide(vec3& pos, vec3& vel) const {
    const float r = 0.72f, BR = BALL_R_CONST;
    const float xW  = H6_CX - H6_S1_W*0.5f;
    const float xE1 = H6_CX + H6_S1_W*0.5f;
    const float xE2 = xW + H6_S2_L;
    const float zS  = H6_TEE_Z;
    const float zJ  = H6_TEE_Z - H6_S1_L;
    const float zN  = zJ - H6_S2_W;
    if(pos.x-BR < xW){ pos.x=xW+BR; if(vel.x<0) vel.x= fabsf(vel.x)*r; }
    if(pos.z+BR > zS){ pos.z=zS-BR; if(vel.z>0) vel.z=-fabsf(vel.z)*r; }
    if(pos.z-BR < zN){ pos.z=zN+BR; if(vel.z<0) vel.z= fabsf(vel.z)*r; }
    if(pos.z<=zJ && pos.x+BR > xE2){ pos.x=xE2-BR; if(vel.x>0) vel.x=-fabsf(vel.x)*r; }
    if(pos.z>zJ  && pos.x+BR > xE1){ pos.x=xE1-BR; if(vel.x>0) vel.x=-fabsf(vel.x)*r; }
    if(pos.x>xE1 && pos.z+BR > zJ) { pos.z=zJ-BR;  if(vel.z>0) vel.z=-fabsf(vel.z)*r; }
}

bool Hole6::nearCup(const vec3& pos) const {
    float cupX = H6_CX - H6_S1_W*0.5f + H6_S2_L - 0.5f;
    float cupZ = H6_TEE_Z - H6_S1_L - H6_S2_W*0.5f;
    float dx=pos.x-cupX, dz=pos.z-cupZ;
    return sqrtf(dx*dx+dz*dz) < 0.35f;
}

vec3 Hole6::getTeePos() const {
    return { H6_CX, BALL_R_CONST, H6_TEE_Z-1.0f };
}
