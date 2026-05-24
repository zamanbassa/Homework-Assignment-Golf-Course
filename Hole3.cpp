#include "Hole3.h"
#include <cmath>

static const float PI3 = 3.14159265358979f;

Hole3::Hole3(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t, const Mesh* circle)
    : Hole(3, q, b, c, t), mCircle(circle)
{
    mWall1 = makeArcWall(H3_R, H3_WTH3, H3_WH3, -PI3/2.f+H3_GAP, -PI3/2.f+2.f*PI3-H3_GAP, 56);
    mWall2 = makeArcWall(H3_R, H3_WTH3, H3_WH3,  PI3/2.f+H3_GAP,  PI3/2.f+2.f*PI3-H3_GAP, 56);
}

Hole3::~Hole3() {
    glDeleteBuffers(1,&mWall1.vbo); glDeleteBuffers(1,&mWall1.ebo); glDeleteVertexArrays(1,&mWall1.vao);
    glDeleteBuffers(1,&mWall2.vbo); glDeleteBuffers(1,&mWall2.ebo); glDeleteVertexArrays(1,&mWall2.vao);
}

void Hole3::render(const mat4& vp) const {
    const float cx=H3_CX, cz=H3_CZ;
    float lc1z = cz + H3_R*0.9f;
    float lc2z = cz - H3_R*0.9f;
    float cupZ = lc2z - H3_R + 0.5f;
    float teeZ = lc1z + H3_R - 0.5f;

    { mat4 m=glm::translate(mat4(1),{cx,0,lc1z}); m=glm::scale(m,{H3_R,1,H3_R}); draw(*mCircle,m,vp,0); }
    { mat4 m=glm::translate(mat4(1),{cx,0,lc2z}); m=glm::scale(m,{H3_R,1,H3_R}); draw(*mCircle,m,vp,0); }
    { mat4 m=glm::translate(mat4(1),{cx,0.005f,teeZ}); m=glm::scale(m,{2.0f,1,1.0f}); draw(*mQuad,m,vp,1); }
    { mat4 m=glm::translate(mat4(1),{cx,0,lc1z}); draw(mWall1,m,vp,21); }
    { mat4 m=glm::translate(mat4(1),{cx,0,lc2z}); draw(mWall2,m,vp,21); }
    { mat4 m=glm::translate(mat4(1),{cx,0,lc1z}); m=glm::scale(m,{H3_OBSR*2,0.40f,H3_OBSR*2}); draw(*mCylinder,m,vp,10); }
    { mat4 m=glm::translate(mat4(1),{cx,0,lc2z}); m=glm::scale(m,{H3_OBSR*2,0.40f,H3_OBSR*2}); draw(*mCylinder,m,vp,10); }
    drawCup(vp, cx, cupZ);
}

void Hole3::wallCollide(vec3& pos, vec3& vel) const {
    const float r = 0.72f, BR = BALL_R_CONST;
    const float cx=H3_CX, cz=H3_CZ;
    float lc1z = cz + H3_R*0.9f;
    float lc2z = cz - H3_R*0.9f;
    float dx = pos.x - cx;
    float dz1 = pos.z - lc1z, dz2 = pos.z - lc2z;
    float d1 = sqrtf(dx*dx+dz1*dz1), d2 = sqrtf(dx*dx+dz2*dz2);
    float maxR = H3_R - BR;
    if(d1 > maxR && d2 > maxR){
        float nx, nz;
        if(d1 <= d2){ nx=dx/d1; nz=dz1/d1; pos.x=cx+nx*maxR; pos.z=lc1z+nz*maxR; }
        else         { nx=dx/d2; nz=dz2/d2; pos.x=cx+nx*maxR; pos.z=lc2z+nz*maxR; }
        float vdn=vel.x*nx+vel.z*nz;
        if(vdn>0){ vel.x-=(1.f+r)*vdn*nx; vel.z-=(1.f+r)*vdn*nz; }
    }
    auto bump = [&](float bx, float bz){
        float dbx=pos.x-bx, dbz=pos.z-bz;
        float db2=dbx*dbx+dbz*dbz, minD=BR+H3_OBSR;
        if(db2 < minD*minD && db2 > 0.0001f){
            float db=sqrtf(db2), push=minD-db;
            float nx=dbx/db, nz=dbz/db;
            pos.x+=nx*push; pos.z+=nz*push;
            float vdn=vel.x*nx+vel.z*nz;
            if(vdn<0){ vel.x-=(1.f+r)*vdn*nx; vel.z-=(1.f+r)*vdn*nz; }
        }
    };
    bump(cx, lc1z); bump(cx, lc2z);
}

bool Hole3::nearCup(const vec3& pos) const {
    float cupZ = (H3_CZ - H3_R*0.9f) - H3_R + 0.5f;
    float dx=pos.x-H3_CX, dz=pos.z-cupZ;
    return sqrtf(dx*dx+dz*dz) < 0.35f;
}

vec3 Hole3::getTeePos() const {
    float teeZ = (H3_CZ + H3_R*0.9f) + H3_R - 0.5f;
    return { H3_CX, BALL_R_CONST, teeZ };
}
