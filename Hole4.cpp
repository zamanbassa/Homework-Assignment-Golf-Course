#include "Hole4.h"
#include <cmath>

Hole4::Hole4(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(4, q, b, c, t)
{
    mFloor   = makeArcFloor(H4_RI, H4_RO, H4_T0, H4_T1, 52);
    mWallIn  = makeArcWall(H4_RI, H4_WTH4, H4_WH4, H4_T0, H4_T1, 52);
    mWallOut = makeArcWall(H4_RO, H4_WTH4, H4_WH4, H4_T0, H4_T1, 52);
}

Hole4::~Hole4() {
    glDeleteBuffers(1,&mFloor.vbo);   glDeleteBuffers(1,&mFloor.ebo);   glDeleteVertexArrays(1,&mFloor.vao);
    glDeleteBuffers(1,&mWallIn.vbo);  glDeleteBuffers(1,&mWallIn.ebo);  glDeleteVertexArrays(1,&mWallIn.vao);
    glDeleteBuffers(1,&mWallOut.vbo); glDeleteBuffers(1,&mWallOut.ebo); glDeleteVertexArrays(1,&mWallOut.vao);
}

void Hole4::render(const mat4& vp) const {
    const float cx=H4_CX, cz=H4_CZ;
    const float WH=H4_WH4, WTH=H4_WTH4;
    const float RMID=(H4_RI+H4_RO)*0.5f;

    { mat4 m=glm::translate(mat4(1),{cx,0,cz}); draw(mFloor,m,vp,0); }
    { mat4 m=glm::translate(mat4(1),{cx+RMID,0.005f,cz}); m=glm::scale(m,{H4_RO-H4_RI,1,1.5f}); draw(*mQuad,m,vp,1); }
    { mat4 m=glm::translate(mat4(1),{cx,0,cz}); draw(mWallIn, m,vp,21); }
    { mat4 m=glm::translate(mat4(1),{cx,0,cz}); draw(mWallOut,m,vp,21); }
    {
        float ct=cosf(H4_T0), st=sinf(H4_T0);
        float ecx=cx+RMID*ct, ecz=cz+RMID*st;
        mat4 m=glm::translate(mat4(1),{ecx,WH*0.5f,ecz});
        m=glm::rotate(m,-H4_T0,{0,1,0});
        m=glm::scale(m,{H4_RO-H4_RI+WTH*2,WH,WTH});
        draw(*mBox,m,vp,21);
    }
    float cx4=cx+RMID*cosf(H4_T0), cz4=cz+RMID*sinf(H4_T0);
    drawCup(vp, cx4, cz4);
}

void Hole4::wallCollide(vec3& pos, vec3& vel) const {
    const float r = 0.72f, BR = BALL_R_CONST;
    float dx=pos.x-H4_CX, dz=pos.z-H4_CZ;
    float d=sqrtf(dx*dx+dz*dz);
    if(d < 0.01f) return;
    float nx=dx/d, nz=dz/d;
    if(d > H4_RO - BR){
        pos.x=H4_CX+nx*(H4_RO-BR); pos.z=H4_CZ+nz*(H4_RO-BR);
        float vdn=vel.x*nx+vel.z*nz;
        if(vdn>0){ vel.x-=(1.f+r)*vdn*nx; vel.z-=(1.f+r)*vdn*nz; }
    }
    if(d < H4_RI + BR){
        pos.x=H4_CX+nx*(H4_RI+BR); pos.z=H4_CZ+nz*(H4_RI+BR);
        float vdn=vel.x*nx+vel.z*nz;
        if(vdn<0){ vel.x-=(1.f+r)*vdn*nx; vel.z-=(1.f+r)*vdn*nz; }
    }
    if(pos.x > H4_CX+H4_RO-BR){ pos.x=H4_CX+H4_RO-BR; if(vel.x>0) vel.x=-fabsf(vel.x)*r; }
    if(pos.z > H4_CZ+BR){ pos.z=H4_CZ+BR; if(vel.z>0) vel.z=-fabsf(vel.z)*r; }
    float cupEndZ = H4_CZ + H4_RO*sinf(H4_T0) + BR;
    if(pos.z < cupEndZ){ pos.z=cupEndZ; if(vel.z<0) vel.z=fabsf(vel.z)*r; }
}

bool Hole4::nearCup(const vec3& pos) const {
    float rmid=(H4_RI+H4_RO)*0.5f;
    float cx4=H4_CX+rmid*cosf(H4_T0), cz4=H4_CZ+rmid*sinf(H4_T0);
    float dx=pos.x-cx4, dz=pos.z-cz4;
    return sqrtf(dx*dx+dz*dz) < 0.35f;
}

vec3 Hole4::getTeePos() const {
    float rmid=(H4_RI+H4_RO)*0.5f;
    return { H4_CX+rmid, BALL_R_CONST, H4_CZ };
}
