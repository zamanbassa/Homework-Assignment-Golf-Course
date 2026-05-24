#include "Hole2.h"
#include <cmath>

Hole2::Hole2(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t, const Mesh* sphere)
    : Hole(2, q, b, c, t), mSphere(sphere) {}

void Hole2::render(const mat4& vp) const {
    const float cx = H2_CX, cz = H2_CZ;
    const float FW = H2_FW, FL = H2_FL;
    const float WH = H2_WH, WTH = H2_WTH;

    { mat4 m=glm::translate(mat4(1),{cx,0.01f,cz}); m=glm::scale(m,{H2_HILL_R,H2_HILL_H,H2_HILL_R}); draw(*mSphere,m,vp,0); }
    { mat4 m=glm::translate(mat4(1),{cx,0,cz}); m=glm::scale(m,{FW,1,FL}); draw(*mQuad,m,vp,0); }
    { mat4 m=glm::translate(mat4(1),{cx,0.005f,cz+5.5f}); m=glm::scale(m,{FW-0.2f,1,1.5f}); draw(*mQuad,m,vp,1); }
    for(int s=-1;s<=1;s+=2){
        mat4 m=glm::translate(mat4(1),{cx+(FW*0.5f+WTH*0.5f)*(float)s,WH*0.5f,cz});
        m=glm::scale(m,{WTH,WH,FL+WTH*2}); draw(*mBox,m,vp,8);
    }
    { mat4 m=glm::translate(mat4(1),{cx,WH*0.5f,cz-FL*0.5f-WTH*0.5f}); m=glm::scale(m,{FW+WTH*2,WH,WTH}); draw(*mBox,m,vp,8); }
    { mat4 m=glm::translate(mat4(1),{cx,WH*0.5f,cz+FL*0.5f+WTH*0.5f}); m=glm::scale(m,{FW+WTH*2,WH,WTH}); draw(*mBox,m,vp,8); }
    drawCup(vp, cx, cz-5.5f);
}

void Hole2::wallCollide(vec3& pos, vec3& vel) const {
    const float r = 0.72f, BR = BALL_R_CONST;
    const float cx = H2_CX, cz = H2_CZ;
    const float hn = cz - H2_FL*0.5f, hs = cz + H2_FL*0.5f;
    const float he = cx + H2_FW*0.5f, hw = cx - H2_FW*0.5f;
    if(pos.x-BR < hw){ pos.x=hw+BR; if(vel.x<0) vel.x= fabsf(vel.x)*r; }
    if(pos.x+BR > he){ pos.x=he-BR; if(vel.x>0) vel.x=-fabsf(vel.x)*r; }
    if(pos.z-BR < hn){ pos.z=hn+BR; if(vel.z<0) vel.z= fabsf(vel.z)*r; }
    if(pos.z+BR > hs){ pos.z=hs-BR; if(vel.z>0) vel.z=-fabsf(vel.z)*r; }
}

float Hole2::groundY(const vec3& pos) const {
    float dx = pos.x - H2_CX, dz = pos.z - H2_CZ;
    float d2 = dx*dx + dz*dz, hr2 = H2_HILL_R*H2_HILL_R;
    if(d2 < hr2){
        float t2 = 1.f - d2/hr2;
        return BALL_R_CONST + 0.01f + H2_HILL_H*sqrtf(t2);
    }
    return BALL_R_CONST;
}

vec3 Hole2::terrainForce(const vec3& pos) const {
    float dx = pos.x - H2_CX, dz = pos.z - H2_CZ;
    float d2 = dx*dx + dz*dz, hr2 = H2_HILL_R*H2_HILL_R;
    if(d2 > 0.0001f && d2 < hr2){
        float d = sqrtf(d2);
        float slope = 9.8f * 2.f * H2_HILL_H * d / hr2;
        return vec3(dx/d * slope, 0.f, dz/d * slope);
    }
    return vec3(0.f);
}

bool Hole2::nearCup(const vec3& pos) const {
    float dx=pos.x-H2_CX, dz=pos.z-(H2_CZ-5.5f);
    return sqrtf(dx*dx+dz*dz) < 0.35f;
}

vec3 Hole2::getTeePos() const {
    return { H2_CX, BALL_R_CONST, H2_CZ+5.5f };
}
