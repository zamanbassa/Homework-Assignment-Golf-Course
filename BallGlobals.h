#ifndef BALLGLOBALS_H
#define BALLGLOBALS_H

// Fairway bounds (half-widths, relative to hole centre)
static const float FW_X = 2.00f;
static const float FW_ZN = -6.50f;
static const float FW_ZS = 6.50f;
static const float BALL_R = 0.08f;
static const float MAX_AIM = 6.0f;

// Hole positions (each hole spans ±7 in Z, ±2 in X from its centre)
static const float HOLE1_X = -33.0f;
static const float HOLE1_Z = 36.0f;
static const float HOLE2_X = -33.0f;
static const float HOLE2_Z = 18.0f;

// Hill for hole 2
static const float HILL_R = 1.5f;
static const float HILL_H = 0.42f;

// Hole 3 — true circular figure-8
static const float HOLE3_X = -33.0f;
static const float HOLE3_Z = -1.0f;
static const float H3_R = 3.5f;     // radius of each circular loop
static const float H3_OBSR = 0.75f; // obstacle cylinder radius

// Hole 4 — banana arc curving left (west) as ball goes north
// Arc sweeps clockwise from tee (east, t=0) to cup (northwest, t=H4_T0)
static const float HOLE4_CX = -37.0f; // centre of curvature
static const float HOLE4_CZ = -11.0f; // z of tee (clear of hole 3)
static const float H4_RI = 7.0f;      // inner fairway radius
static const float H4_RO = 11.0f;     // outer fairway radius  (width = 4)
static const float H4_T0 = -1.41f;    // cup-end angle  ≈ -81°  (northwest)
static const float H4_T1 = 0.0f;      // tee-end angle  = 0°    (east)

// Hole 5 — regular pentagon, tee at south vertex, cup at north-edge midpoint
static const float HOLE5_CX = -33.0f;
static const float HOLE5_CZ = -31.0f; // centre; tee ≈ z=-25, cup ≈ z=-36
static const float H5_R = 6.0f;       // circumradius

// Hole 6 — L-shape (short N-S arm heading north, long E-W arm going east at top)
static const float H6_CX = -33.0f;    // x-centre of short arm (aligns with holes 1-5)
static const float H6_TEE_Z = -38.0f; // south end of short arm (fairway boundary, tee side)
static const float H6_S1_W = 4.0f;    // short arm width (x)
static const float H6_S1_L = 6.5f;    // short arm length (z, going north)
static const float H6_S2_W = 4.0f;    // long arm width (z)
static const float H6_S2_L = 16.0f;   // long arm length (x, going east)

// Hole 7 — S-shape using two connected semicircle arcs
static const float H7_Z0 = -46.5f; // centerline z
static const float H7_X0 = -16.0f; // tee x (east of hole 6)
static const float H7_RA = 5.0f;   // arc radius (center of fairway)
static const float H7_LS = 3.0f;   // straight sections at tee and cup ends
static const float H7_FW = 4.0f;   // fairway width (RI=3, RO=7)

// Hole 8 — triangle, flat base west, apex east
static const float H8_BX = 12.0f; // base x (entry, just east of hole 7)
static const float H8_CZ = H7_Z0; // same z centreline as hole 7
static const float H8_HW = 3.5f;  // half-width at base
static const float H8_LEN = 9.0f; // length base→apex

static int gCurrentHole = 1;

#endif