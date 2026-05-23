#include "Holes.h"

// ─── draw() shorthand ────────────────────────────────────────────────────────
void draw(const Mesh &m, const mat4 &model, const mat4 &vp, int surf)
{
    mat4 mvp = vp * model;
    glUniformMatrix4fv(glGetUniformLocation(gProg, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(glGetUniformLocation(gProg, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(glGetUniformLocation(gProg, "uSurface"), surf);
    glBindVertexArray(m.VAO());
    glDrawElements(GL_TRIANGLES, m.Count(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// ─── Draw a golf hole centred at (cx, 0, cz) ─────────────────────────────────
void drawHole(const mat4 &vp, float cx, float cz)
{
    const float FW = 4.0f;
    const float FL = 14.0f;
    const float WH = 0.45f;
    const float WTH = 0.30f;
    const float diagLen = FL * 0.6;
    const float strLen = FL * 0.4;
    const float diagAngle = radians(120.0f);

    float joinX = cx;
    float joinZ = cz - strLen;

    // shift cup back for diagonal
    float endX = joinX + sinf(diagAngle) * diagLen;
    float endZ = joinZ - cosf(diagAngle) * diagLen;

    // Rotate entire hole 90° around its centre
    mat4 R = glm::translate(mat4(1), {cx, 0, cz}) * glm::rotate(mat4(1), PI * 0.5f, {0, 1, 0}) * glm::translate(mat4(1), {-cx, 0, -cz});

    // Fairway
    {
        // diagonal
        {
            mat4 m = glm::translate(mat4(1), {joinX + sinf(diagAngle) * diagLen * 0.5f, 0, joinZ - cosf(diagAngle) * diagLen * 0.5f});
            m = glm::rotate(m, -diagAngle, {0, 1, 0});
            m = glm::scale(m, {FW, 1, diagLen});
            draw(mQuad, R * m, vp, 0);
        }

        // straight
        {
            mat4 m = glm::translate(mat4(1), {cx, 0, cz});
            m = glm::scale(m, {FW, 1, FL});
            draw(mQuad, R * m, vp, 0);
        }
    }
    // Tee box (south end)
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.005f, cz + 5.5f});
        m = glm::scale(m, {FW - 0.2f, 1, 1.5f});
        draw(mQuad, R * m, vp, 1);
    }

    // Diagonal Side Walls
    for (int s = -1; s <= 1; s += 2)
    {
        mat4 m = glm::translate(mat4(1), {joinX + sinf(diagAngle) * diagLen * 0.5f + cosf(diagAngle) * (FW * 0.5f + WTH * 0.5f) * (float)s, WH * 0.5f, joinZ - cosf(diagAngle) * diagLen * 0.5f - sinf(diagAngle) * (FW * 0.5f + WTH * 0.5f) * (float)s});
        m = glm::rotate(m, -diagAngle, {0, 1, 0});
        m = glm::scale(m, {WTH, WH, diagLen});
        draw(mBox, R * m, vp, 8);
    }

    // Straight Side walls
    for (int s = -1; s <= 1; s += 2)
    {
        mat4 m = glm::translate(mat4(1), {cx + (FW * 0.5f + WTH * 0.5f) * (float)s, WH * 0.5f, cz});
        m = glm::scale(m, {WTH, WH, FL + WTH * 2});
        draw(mBox, R * m, vp, 8);
    }

    // North end wall
    {
        mat4 m = glm::translate(mat4(1), {cx, WH * 0.5f, cz - FL * 0.5f - WTH * 0.5f});
        m = glm::scale(m, {FW + WTH * 2, WH, WTH});
        draw(mBox, R * m, vp, 8);
    }
    // South end wall
    {
        mat4 m = glm::translate(mat4(1), {cx, WH * 0.5f, cz + FL * 0.5f + WTH * 0.5f});
        m = glm::scale(m, {FW + WTH * 2, WH, WTH});
        draw(mBox, R * m, vp, 8);
    }
    // Cup torus + disc (north end = cup)
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.01f, cz - 5.5f});
        m = glm::scale(m, {0.35f, 0.1f, 0.35f});
        draw(mTorus, R * m, vp, 19);
    }
    {
        mat4 m = glm::translate(mat4(1), {cx, 0.001f, cz - 5.5f});
        m = glm::scale(m, {0.33f, 1, 0.33f});
        draw(mQuad, R * m, vp, 19);
    }
    // Flag pole
    {
        mat4 m = glm::translate(mat4(1), {cx + 0.38f, 0, cz - 5.5f});
        m = glm::scale(m, {0.05f, 2.2f, 0.05f});
        draw(mCylinder, R * m, vp, 16);
    }
    // Flag pennant — vertical
    {
        mat4 m = glm::translate(mat4(1), {cx + 0.655f, 2.0f, cz - 5.5f});
        m = glm::rotate(m, PI * 0.5f, {1, 0, 0});
        m = glm::scale(m, {0.55f, 1.0f, 0.32f});
        draw(mQuad, R * m, vp, 17);
    }
}

// ─── Hole 2 — same layout, hill instead of water+bridge ──────────────────────
void drawHole2(const mat4 &vp, float cx, float cz)
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
void drawHole3(const mat4 &vp, float cx, float cz)
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
void drawHole4(const mat4 &vp)
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
void drawHole5(const mat4 &vp)
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
void drawHole6(const mat4 &vp)
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
void drawHole7(const mat4 &vp)
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
void drawHole8(const mat4 &vp)
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
