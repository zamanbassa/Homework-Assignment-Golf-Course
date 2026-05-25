#include "Hole5.h"
#include <cmath>

static const float PI5     = 3.14159265358979f;
static const float H5_OBSR = 1.0f;   // central bumper post radius

Hole5::Hole5(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(5, q, b, c, t)
{
    mFloor = makePentagonFloor();
}

Hole5::~Hole5() {
    glDeleteBuffers(1,&mFloor.vbo); glDeleteBuffers(1,&mFloor.ebo); glDeleteVertexArrays(1,&mFloor.vao);
}

void Hole5::render(const mat4& vp) const {
    const float cx=H5_CX, cz=H5_CZ;
    const float WH=H5_WH5, WTH=H5_WTH5;

    { mat4 m=glm::translate(mat4(1),{cx,0,cz}); m=glm::scale(m,{H5_R,1,H5_R}); draw(mFloor,m,vp,0); }
    { mat4 m=glm::translate(mat4(1),{cx,0.005f,cz+H5_R-1.8f}); m=glm::scale(m,{2.0f,1,1.0f}); draw(*mQuad,m,vp,1); }
    float edgeLen = 2.0f*H5_R*sinf(PI5/5.0f);
    for(int k=0;k<5;k++){
        float t0=PI5/2.f+k*(2.f*PI5/5.f), t1=PI5/2.f+(k+1)*(2.f*PI5/5.f);
        float x0=H5_R*cosf(t0), z0=H5_R*sinf(t0);
        float x1=H5_R*cosf(t1), z1=H5_R*sinf(t1);
        float mx=(x0+x1)*0.5f, mz=(z0+z1)*0.5f;
        float edx=x1-x0, edz=z1-z0;
        float ang=atan2f(-edz,edx);
        drawWall(vp, cx+mx, WH*0.5f, cz+mz, edgeLen+WTH, WH, WTH, ang);
    }
    // bumper
    { mat4 m = glm::translate(mat4(1),{cx, 0.f, cz});
      m = glm::scale(m, {H5_OBSR*2.f, 0.45f, H5_OBSR*2.f}); draw(*mCylinder, m, vp, 10); }
    float t2=PI5/2.f+2.f*(2.f*PI5/5.f), t3=PI5/2.f+3.f*(2.f*PI5/5.f);
    float cupX=cx+H5_R*(cosf(t2)+cosf(t3))*0.5f;
    float cupZ=cz+H5_R*(sinf(t2)+sinf(t3))*0.5f+1.2f;
    drawCup(vp, cupX, cupZ);
}

void Hole5::wallCollide(vec3& pos, vec3& vel) const {
    const float r = 0.72f, BR = BALL_R_CONST;
    // Central bumper post
    {
        float dbx = pos.x - H5_CX, dbz = pos.z - H5_CZ;
        float db2 = dbx*dbx + dbz*dbz, minD = BR + H5_OBSR;
        if(db2 < minD*minD && db2 > 0.0001f){
            float db = sqrtf(db2), push = minD - db;
            float nx = dbx/db, nz = dbz/db;
            pos.x += nx*push; pos.z += nz*push;
            float vdn = vel.x*nx + vel.z*nz;
            if(vdn < 0){ vel.x -= (1.f+r)*vdn*nx; vel.z -= (1.f+r)*vdn*nz; }
        }
    }
    for(int k=0;k<5;k++){
        float t0=PI5/2.f+k*(2.f*PI5/5.f), t1=PI5/2.f+(k+1)*(2.f*PI5/5.f);
        float ax=H5_CX+H5_R*cosf(t0), az=H5_CZ+H5_R*sinf(t0);
        float bx=H5_CX+H5_R*cosf(t1), bz=H5_CZ+H5_R*sinf(t1);
        float edx=bx-ax, edz=bz-az;
        float len=sqrtf(edx*edx+edz*edz);
        float onx=edz/len, onz=-edx/len;
        float d=(pos.x-ax)*onx+(pos.z-az)*onz;
        if(d > -BR){
            float pen=d+BR;
            pos.x-=onx*pen; pos.z-=onz*pen;
            float vdn=vel.x*onx+vel.z*onz;
            if(vdn>0){ vel.x-=(1.f+r)*vdn*onx; vel.z-=(1.f+r)*vdn*onz; }
        }
    }
}

bool Hole5::nearCup(const vec3& pos) const {
    float t2=PI5/2.f+2.f*(2.f*PI5/5.f), t3=PI5/2.f+3.f*(2.f*PI5/5.f);
    float cupX=H5_CX+H5_R*(cosf(t2)+cosf(t3))*0.5f;
    float cupZ=H5_CZ+H5_R*(sinf(t2)+sinf(t3))*0.5f+1.2f;
    float dx=pos.x-cupX, dz=pos.z-cupZ;
    return sqrtf(dx*dx+dz*dz) < 0.35f;
}

vec3 Hole5::getTeePos() const {
    return { H5_CX, BALL_R_CONST, H5_CZ+H5_R-1.5f };
}
