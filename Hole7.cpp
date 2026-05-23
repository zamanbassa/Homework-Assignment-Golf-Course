#include "Hole7.h"
#include <cmath>

static const float PI7 = 3.14159265358979f;

Hole7::Hole7(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(7, q, b, c, t)
{
    float ri = H7_RA - H7_FW*0.5f;
    float ro = H7_RA + H7_FW*0.5f;
    mFloor1   = makeArcFloor(ri, ro, PI7,       0.0f,    48);
    mWall1In  = makeArcWall (ri, H7_WTH7, H7_WH7, PI7, 0.0f,    48);
    mWall1Out = makeArcWall (ro, H7_WTH7, H7_WH7, PI7, 0.0f,    48);
    mFloor2   = makeArcFloor(ri, ro, PI7,       2.f*PI7, 48);
    mWall2In  = makeArcWall (ri, H7_WTH7, H7_WH7, PI7, 2.f*PI7, 48);
    mWall2Out = makeArcWall (ro, H7_WTH7, H7_WH7, PI7, 2.f*PI7, 48);
}

Hole7::~Hole7() {
    auto fm=[](Mesh& m){ glDeleteBuffers(1,&m.vbo); glDeleteBuffers(1,&m.ebo); glDeleteVertexArrays(1,&m.vao); };
    fm(mFloor1); fm(mFloor2);
    fm(mWall1In); fm(mWall1Out);
    fm(mWall2In); fm(mWall2Out);
}

void Hole7::render(const mat4& vp) const {
    const float ri = H7_RA-H7_FW*0.5f, ro = H7_RA+H7_FW*0.5f;
    const float c1x = H7_X0+H7_LS+H7_RA;
    const float c2x = H7_X0+H7_LS+3.f*H7_RA;
    const float cz  = H7_Z0;
    const float teeX= c1x - H7_RA;
    const float cupX= c2x + H7_RA;

    { mat4 m=glm::translate(mat4(1),{c1x,0,cz}); draw(mFloor1,m,vp,0); }
    { mat4 m=glm::translate(mat4(1),{c2x,0,cz}); draw(mFloor2,m,vp,0); }
    { mat4 m=glm::translate(mat4(1),{teeX+1.f,0.005f,cz+1.f}); m=glm::scale(m,{ro-ri-0.3f,1,1.5f}); draw(*mQuad,m,vp,1); }
    { mat4 m=glm::translate(mat4(1),{c1x,0,cz}); draw(mWall1In,m,vp,8); draw(mWall1Out,m,vp,8); }
    { mat4 m=glm::translate(mat4(1),{c2x,0,cz}); draw(mWall2In,m,vp,8); draw(mWall2Out,m,vp,8); }
    drawWall(vp, teeX, H7_WH7*0.5f, cz, ro-ri+H7_WTH7, H7_WH7, H7_WTH7);
    drawWall(vp, cupX, H7_WH7*0.5f, cz, ro-ri+H7_WTH7, H7_WH7, H7_WTH7);
    float cx9=cupX-1.f, cz9=cz-1.f;
    drawCup(vp, cx9, cz9);
}

void Hole7::wallCollide(vec3& pos, vec3& vel) const {
    const float r=0.72f, BR=BALL_R_CONST;
    const float ri=H7_RA-H7_FW*0.5f, ro=H7_RA+H7_FW*0.5f;
    const float c1x=H7_X0+H7_LS+H7_RA, c2x=H7_X0+H7_LS+3.f*H7_RA;
    if(pos.x < c1x-ro+BR){ pos.x=c1x-ro+BR; if(vel.x<0) vel.x=fabsf(vel.x)*r; }
    if(pos.x > c2x+ro-BR){ pos.x=c2x+ro-BR; if(vel.x>0) vel.x=-fabsf(vel.x)*r; }
    float midX=(c1x+c2x)*0.5f;
    float cx=(pos.x<=midX)?c1x:c2x;
    float dx=pos.x-cx, dz=pos.z-H7_Z0;
    float d=sqrtf(dx*dx+dz*dz);
    if(d>0.01f){
        float nx=dx/d, nz=dz/d;
        if(d>ro-BR){ pos.x=cx+nx*(ro-BR); pos.z=H7_Z0+nz*(ro-BR);
            float vdn=vel.x*nx+vel.z*nz; if(vdn>0){vel.x-=(1.f+r)*vdn*nx; vel.z-=(1.f+r)*vdn*nz;} }
        if(d<ri+BR){ pos.x=cx+nx*(ri+BR); pos.z=H7_Z0+nz*(ri+BR);
            float vdn=vel.x*nx+vel.z*nz; if(vdn<0){vel.x-=(1.f+r)*vdn*nx; vel.z-=(1.f+r)*vdn*nz;} }
    }
}

bool Hole7::nearCup(const vec3& pos) const {
    float cupX=H7_X0+H7_LS+4.f*H7_RA-1.f;
    float cupZ=H7_Z0-1.f;
    float dx=pos.x-cupX, dz=pos.z-cupZ;
    return sqrtf(dx*dx+dz*dz) < 0.35f;
}

vec3 Hole7::getTeePos() const {
    return { H7_X0+H7_LS+1.f, BALL_R_CONST, H7_Z0+1.f };
}
