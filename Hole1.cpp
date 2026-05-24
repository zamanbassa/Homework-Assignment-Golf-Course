#include "Hole1.h"
#include <cmath>

Hole1::Hole1(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(1, q, b, c, t) {}

void Hole1::render(const mat4& vp) const {
    const float cx = H1_CX, cz = H1_CZ;
    const float FW = H1_FW, FL = H1_FL;
    const float WH = H1_WH, WTH = H1_WTH;

    { mat4 m=glm::translate(mat4(1),{cx,0,cz}); m=glm::scale(m,{FW,1,FL}); draw(*mQuad,m,vp,0); }
    { mat4 m=glm::translate(mat4(1),{cx,0.005f,cz+5.5f}); m=glm::scale(m,{FW-0.2f,1,1.5f}); draw(*mQuad,m,vp,1); }
    { mat4 m=glm::translate(mat4(1),{cx,-0.01f,cz}); m=glm::scale(m,{FW,1,2.2f}); draw(*mQuad,m,vp,5); }
    { mat4 m=glm::translate(mat4(1),{cx,0.06f,cz}); m=glm::scale(m,{1.9f,0.14f,2.5f}); draw(*mBox,m,vp,6); }
    for(int s=-1;s<=1;s+=2){
        mat4 r=glm::translate(mat4(1),{cx+(float)s*0.85f,0.26f,cz}); r=glm::scale(r,{0.09f,0.45f,2.5f}); draw(*mBox,r,vp,6);
    }
    for(int s=-1;s<=1;s+=2){
        mat4 m=glm::translate(mat4(1),{cx+(FW*0.5f+WTH*0.5f)*(float)s,WH*0.5f,cz});
        m=glm::scale(m,{WTH,WH,FL+WTH*2}); draw(*mBox,m,vp,21);
    }
    { mat4 m=glm::translate(mat4(1),{cx,WH*0.5f,cz-FL*0.5f-WTH*0.5f}); m=glm::scale(m,{FW+WTH*2,WH,WTH}); draw(*mBox,m,vp,8); }
    { mat4 m=glm::translate(mat4(1),{cx,WH*0.5f,cz+FL*0.5f+WTH*0.5f}); m=glm::scale(m,{FW+WTH*2,WH,WTH}); draw(*mBox,m,vp,8); }
    drawCup(vp, cx, cz-5.5f);
}

void Hole1::wallCollide(vec3& pos, vec3& vel) const {
    const float r = 0.72f, BR = BALL_R_CONST;
    const float cx = H1_CX, cz = H1_CZ;
    const float hn = cz - H1_FL*0.5f, hs = cz + H1_FL*0.5f;
    const float he = cx + H1_FW*0.5f, hw = cx - H1_FW*0.5f;
    if(pos.x-BR < hw){ pos.x=hw+BR; if(vel.x<0) vel.x= fabsf(vel.x)*r; }
    if(pos.x+BR > he){ pos.x=he-BR; if(vel.x>0) vel.x=-fabsf(vel.x)*r; }
    if(pos.z-BR < hn){ pos.z=hn+BR; if(vel.z<0) vel.z= fabsf(vel.z)*r; }
    if(pos.z+BR > hs){ pos.z=hs-BR; if(vel.z>0) vel.z=-fabsf(vel.z)*r; }
}

bool Hole1::nearCup(const vec3& pos) const {
    float dx=pos.x-H1_CX, dz=pos.z-(H1_CZ-5.5f);
    return sqrtf(dx*dx+dz*dz) < 0.35f;
}

vec3 Hole1::getTeePos() const {
    return { H1_CX, BALL_R_CONST, H1_CZ+5.5f };
}
