#include "Hole8.h"
#include <cmath>

Hole8::Hole8(const Mesh* q, const Mesh* b, const Mesh* c, const Mesh* t)
    : Hole(8, q, b, c, t)
{
    mFloor = makeTriFloor({0,0,-H8_HW}, {0,0,H8_HW}, {H8_LEN,0,0});
}

Hole8::~Hole8() {
    glDeleteBuffers(1,&mFloor.vbo);
    glDeleteBuffers(1,&mFloor.ebo);
    glDeleteVertexArrays(1,&mFloor.vao);
}

void Hole8::render(const mat4& vp) const {
    const float bx=H8_BX, cz=H8_CZ, hw=H8_HW, len=H8_LEN;
    const float WH=H8_WH8, WTH=H8_WTH8;
    const float diagLen=sqrtf(len*len + hw*hw);

    // Triangle floor: base at (bx, cz±hw), apex at (bx+len, cz)
    { mat4 m=glm::translate(mat4(1),{bx,0,cz}); draw(mFloor,m,vp,0); }

    // Tee mat near base (west entry)
    { mat4 m=glm::translate(mat4(1),{bx+0.8f,0.005f,cz});
      m=glm::scale(m,{1.5f,1,hw}); draw(*mQuad,m,vp,1); }

    // Base wall (west)
    drawWall(vp, bx, WH*0.5f, cz, WTH, WH, 2*hw+WTH);

    // South diagonal: base-south (bx, cz+hw) → apex (bx+len, cz)
    // edge dir (len, -hw) → angle atan2f(hw, len)
    { float mx=(2*bx+len)*0.5f, mz=(2*cz+hw)*0.5f;
      drawWall(vp, mx, WH*0.5f, mz, diagLen+WTH, WH, WTH, atan2f(hw,len)); }

    // North diagonal: base-north (bx, cz-hw) → apex (bx+len, cz)
    // edge dir (len, hw) → angle atan2f(-hw, len)
    { float mx=(2*bx+len)*0.5f, mz=(2*cz-hw)*0.5f;
      drawWall(vp, mx, WH*0.5f, mz, diagLen+WTH, WH, WTH, atan2f(-hw,len)); }

    // Cup near apex (east)
    drawCup(vp, bx+len-1.5f, cz);
}

void Hole8::wallCollide(vec3& pos, vec3& vel) const {
    const float r=0.72f, BR=BALL_R_CONST;
    const float bx=H8_BX, cz=H8_CZ, hw=H8_HW, len=H8_LEN;
    const float diagLen=sqrtf(len*len + hw*hw);

    // West cap (base)
    if(pos.x < bx+BR){ pos.x=bx+BR; if(vel.x<0) vel.x=fabsf(vel.x)*r; }
    // East cap (apex)
    if(pos.x > bx+len-BR){ pos.x=bx+len-BR; if(vel.x>0) vel.x=-fabsf(vel.x)*r; }

    // South wall: base-south (bx, cz+hw) → apex (bx+len, cz)
    // inward normal: (-hw, -len)/diagLen
    {
        float nx=-hw/diagLen, nz=-len/diagLen;
        float d=(pos.x-bx)*nx+(pos.z-(cz+hw))*nz;
        if(d < BR){
            float push=BR-d;
            pos.x+=push*nx; pos.z+=push*nz;
            float vn=vel.x*nx+vel.z*nz;
            if(vn<0){ vel.x-=(1.f+r)*vn*nx; vel.z-=(1.f+r)*vn*nz; }
        }
    }
    // North wall: base-north (bx, cz-hw) → apex (bx+len, cz)
    // inward normal: (-hw, len)/diagLen
    {
        float nx=-hw/diagLen, nz=len/diagLen;
        float d=(pos.x-bx)*nx+(pos.z-(cz-hw))*nz;
        if(d < BR){
            float push=BR-d;
            pos.x+=push*nx; pos.z+=push*nz;
            float vn=vel.x*nx+vel.z*nz;
            if(vn<0){ vel.x-=(1.f+r)*vn*nx; vel.z-=(1.f+r)*vn*nz; }
        }
    }
}

bool Hole8::nearCup(const vec3& pos) const {
    float cupX=H8_BX+H8_LEN-1.5f;
    float dx=pos.x-cupX, dz=pos.z-H8_CZ;
    return sqrtf(dx*dx+dz*dz) < 0.35f;
}

vec3 Hole8::getTeePos() const {
    return { H8_BX+0.8f, BALL_R_CONST, H8_CZ };
}
