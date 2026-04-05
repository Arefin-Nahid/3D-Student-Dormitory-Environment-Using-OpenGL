/*
 * scene_buildings.cpp
 *
 * Layout matches the reference image:
 *   – TWO PARALLEL BUILDINGS, left and right of a central courtyard
 *     LEFT  BUILDING  : x = -22 .. -10   (student living, 2 floors)
 *     RIGHT BUILDING  : x =  10 ..  22   (common facilities, 2 floors)
 *     COURTYARD       : x = -10 ..  10   (grass, benches, trees, central staircase)
 *   – Front gate at z = +28 with brick pillars and iron gate
 *   – Both buildings 2-storey, brick base + concrete upper, flat roof with overhang
 *   – Central raised staircase at the back of the courtyard (z ≈ -12)
 *   – Left building interior: corridor + student rooms (bed, desk, wardrobe)
 *   – Right building interior: dining hall (floor 1) + reading room / common room (floor 0)
 *
 * All functionality (lighting toggles, texture modes, viewport, O-room) unchanged.
 */

#include "scene_buildings.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// ── Shared material palette (matches reference image) ────────────────────────
static const glm::vec3 BRICK  (0.62f, 0.28f, 0.18f);   // red brick lower storey
static const glm::vec3 CONC   (0.80f, 0.80f, 0.78f);   // concrete upper / walls
static const glm::vec3 ROOF_C (0.68f, 0.68f, 0.66f);   // flat roof
static const glm::vec3 TRIM   (0.55f, 0.55f, 0.53f);   // darker roof trim / slab edge
static const glm::vec3 CREAM  (0.94f, 0.91f, 0.84f);   // interior walls
static const glm::vec3 FLOOR_C(0.78f, 0.72f, 0.64f);   // tile floor
static const glm::vec3 WOOD_D (0.36f, 0.22f, 0.10f);   // dark wood (doors, furniture)
static const glm::vec3 WOOD_M (0.58f, 0.40f, 0.20f);   // medium wood (desk / table)
static const glm::vec3 WOOD_L (0.74f, 0.56f, 0.32f);   // light oak
static const glm::vec3 IRON   (0.22f, 0.22f, 0.24f);   // iron railings / gate
static const glm::vec3 WARM_W (1.00f, 0.96f, 0.82f);   // warm white (bulb)
static const glm::vec3 SOFA_C (0.72f, 0.64f, 0.52f);   // beige sofa
static const glm::vec3 PLANT  (0.20f, 0.52f, 0.18f);   // indoor plant green
static const glm::vec3 ZERO3  (0.f, 0.f, 0.f);

// ════════════════════════════════════════════════════════════════════════════
//  HELPERS
// ════════════════════════════════════════════════════════════════════════════

// Wall-mounted exterior sconce light
static void wallLight(unsigned int sh, float x, float y, float z, int faceDir) {
    // faceDir: +1 = face +x, -1 = face -x
    glm::mat4 m;
    // Backplate
    m = glm::scale(glm::translate(glm::mat4(1.f), {x, y, z}), {0.08f, 0.22f, 0.22f});
    drawCube(sh, m, TRIM);
    // Arm
    float armX = x + faceDir * 0.18f;
    m = glm::scale(glm::translate(glm::mat4(1.f), {armX, y, z}), {0.30f, 0.06f, 0.06f});
    drawCube(sh, m, TRIM);
    // Lantern
    m = glm::scale(glm::translate(glm::mat4(1.f), {x + faceDir * 0.34f, y - 0.12f, z}), {0.14f, 0.22f, 0.14f});
    drawCylinder(sh, m, glm::vec3(0.88f, 0.84f, 0.55f));
    if (!dayMode) {
        glm::vec3 em(0.90f, 0.82f, 0.45f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(em));
        m = glm::scale(glm::translate(glm::mat4(1.f), {x + faceDir * 0.34f, y - 0.12f, z}), {0.10f, 0.16f, 0.10f});
        drawSphere(sh, m, WARM_W, texMarble, 1.f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    }
}

// Corridor pendant ceiling light
static void pendantLight(unsigned int sh, float x, float y, float z) {
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), {x, y, z}), {0.24f, 0.04f, 0.24f});
    drawCylinder(sh, m, TRIM);
    m = glm::scale(glm::translate(glm::mat4(1.f), {x, y - 0.22f, z}), {0.03f, 0.38f, 0.03f});
    drawCylinder(sh, m, IRON);
    {
        glm::mat4 sh2 = glm::translate(glm::mat4(1.f), {x, y - 0.44f, z});
        sh2 = glm::rotate(sh2, glm::radians(180.f), {1.f, 0.f, 0.f});
        sh2 = glm::scale(sh2, {0.32f, 0.28f, 0.32f});
        drawCone(sh, sh2, glm::vec3(0.72f, 0.68f, 0.58f));
    }
    glm::vec3 bulb = WARM_W;
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(bulb * 0.85f));
    m = glm::scale(glm::translate(glm::mat4(1.f), {x, y - 0.40f, z}), {0.10f, 0.10f, 0.10f});
    drawSphere(sh, m, bulb, texMarble, 1.f);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
}

// Drain pipe on exterior wall
static void drainPipe(unsigned int sh, float x, float z, float h) {
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), {x, h * 0.5f, z}), {0.10f, h, 0.10f});
    drawCylinder(sh, m, glm::vec3(0.45f, 0.45f, 0.46f));
}

// Simple round decorative courtyard tree (not palm / cone)
static void decorTree(unsigned int sh, float x, float z) {
    glm::mat4 m;
    // Pot / planter box
    m = glm::scale(glm::translate(glm::mat4(1.f), {x, 0.22f, z}), {1.10f, 0.44f, 1.10f});
    drawCube(sh, m, BRICK, texBrick, 1.f);
    // Trunk
    m = glm::scale(glm::translate(glm::mat4(1.f), {x, 1.20f, z}), {0.20f, 1.60f, 0.20f});
    drawCylinder(sh, m, COL_TRUNK, texWood, 1.f);
    // Canopy sphere
    m = glm::scale(glm::translate(glm::mat4(1.f), {x, 2.85f, z}), {1.80f, 1.80f, 1.80f});
    drawSphere(sh, m, glm::vec3(0.25f, 0.52f, 0.18f), texGrass, 2.f);
}

// ════════════════════════════════════════════════════════════════════════════
//  LEFT BUILDING  –  Student Living Wing
//  World: x = -22 .. -10,  z = -16 .. +16,  2 floors (each 3.6 units)
// ════════════════════════════════════════════════════════════════════════════
void renderDormBlockA(unsigned int sh) {
    const float X0 = -22.f, X1 = -10.f, XC = (X0 + X1) * 0.5f;
    const float Z0 = -16.f, Z1 =  16.f, ZC = (Z0 + Z1) * 0.5f;
    const float W  = X1 - X0;   // 12
    const float D  = Z1 - Z0;   // 32
    const float FH = 3.6f;      // floor height
    const float TH = FH * 2.f;  // total height = 7.2
    const float WALL_T = 0.35f;

    glm::mat4 m;

    // ── EXTERIOR SHELL ────────────────────────────────────────────────────

    // Ground-floor brick band (lower 1.8 of 3.6)
    m = glm::scale(glm::translate(glm::mat4(1.f), {XC, FH * 0.25f, ZC}), {W, FH * 0.5f, D});
    drawCube(sh, m, BRICK, texBrick, 8.f);

    // Upper-floor concrete
    m = glm::scale(glm::translate(glm::mat4(1.f), {XC, FH + FH * 0.5f, ZC}), {W, FH, D});
    drawCube(sh, m, CONC, texConcrete, 8.f);

    // Floor slab band between storeys
    m = glm::scale(glm::translate(glm::mat4(1.f), {XC, FH, ZC}), {W + 0.1f, 0.28f, D + 0.1f});
    drawCube(sh, m, TRIM, texConcrete, 5.f);

    // Flat roof with overhang
    m = glm::scale(glm::translate(glm::mat4(1.f), {XC, TH + 0.18f, ZC}), {W + 0.8f, 0.38f, D + 0.8f});
    drawCube(sh, m, ROOF_C, texConcrete, 6.f);
    // Roof parapet
    m = glm::scale(glm::translate(glm::mat4(1.f), {XC, TH + 0.46f, ZC}), {W + 0.82f, 0.35f, D + 0.82f});
    drawCube(sh, m, TRIM, texConcrete, 6.f);

    // ── RIGHT FACE (courtyard-facing, x = X1) – windows both floors ──────
    for (int fl = 0; fl < 2; fl++) {
        float fy = fl * FH;
        // 4 windows per floor
        for (int i = 0; i < 4; i++) {
            float wz = Z0 + 4.f + i * 6.8f;
            m = glm::scale(glm::translate(glm::mat4(1.f), {X1 - 0.01f, fy + FH * 0.56f, wz}), {WALL_T + 0.04f, 1.80f, 2.60f});
            drawCrystalGlass(sh, m);
            // Lintel
            m = glm::scale(glm::translate(glm::mat4(1.f), {X1 - 0.01f, fy + FH * 0.56f + 0.98f, wz}), {WALL_T + 0.06f, 0.22f, 2.80f});
            drawCube(sh, m, TRIM, texConcrete, 1.f);
            // Sill
            m = glm::scale(glm::translate(glm::mat4(1.f), {X1 - 0.01f, fy + FH * 0.56f - 1.00f, wz}), {WALL_T + 0.06f, 0.14f, 2.82f});
            drawCube(sh, m, CONC, texConcrete, 1.f);
        }
        // Exterior wall lights (right face, between windows)
        for (int i = 0; i < 3; i++) {
            float wz = Z0 + 7.5f + i * 6.8f;
            wallLight(sh, X1 + 0.04f, fy + FH * 0.72f, wz, +1);
        }
    }

    // ── LEFT / OUTER face (x = X0) – smaller windows ─────────────────────
    for (int fl = 0; fl < 2; fl++) {
        float fy = fl * FH;
        for (int i = 0; i < 3; i++) {
            float wz = Z0 + 5.f + i * 9.0f;
            m = glm::scale(glm::translate(glm::mat4(1.f), {X0 + 0.01f, fy + FH * 0.56f, wz}), {WALL_T + 0.04f, 1.60f, 2.20f});
            drawGlass(sh, m);
        }
    }

    // Drain pipes on outer face
    drainPipe(sh, X0 - 0.04f, Z0 + 2.f,  TH + 0.5f);
    drainPipe(sh, X0 - 0.04f, Z0 + 16.f, TH + 0.5f);

    // ── GROUND FLOOR INTERIOR – corridor + 2 student rooms ───────────────
    {
        const float FY = 0.62f;
        const float CLG = FH - 0.38f;

        // Corridor strip (x = X1 - 2.0 .. X1, full z depth)
        float CX1 = X1 - 2.5f, CXC = (CX1 + X1) * 0.5f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {CXC, FY - 0.06f, ZC}), {X1 - CX1, 0.10f, D});
        drawCube(sh, m, FLOOR_C, texTile, 10.f);
        // Corridor ceiling with pendant lights
        m = glm::scale(glm::translate(glm::mat4(1.f), {CXC, FY + CLG + 0.06f, ZC}), {X1 - CX1, 0.10f, D});
        drawCube(sh, m, CREAM, texConcrete, 4.f);
        for (int i = 0; i < 5; i++) {
            float pz = Z0 + 3.5f + i * 6.0f;
            pendantLight(sh, CXC, FY + CLG - 0.02f, pz);
        }
        // Exit/emergency sign (green box on corridor wall)
        {
            glm::vec3 em(0.10f, 0.90f, 0.25f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(em));
            m = glm::scale(glm::translate(glm::mat4(1.f), {CX1 + 0.02f, FY + CLG * 0.80f, Z0 + 6.5f}), {0.05f, 0.22f, 0.38f});
            drawCube(sh, m, em);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
        }

        // Room 1 (south half)
        float R1_X1 = X0 + WALL_T, R1_X2 = CX1 - WALL_T;
        float R1_Z1 = Z0 + WALL_T, R1_Z2 = ZC - WALL_T;
        float R1_XC = (R1_X1 + R1_X2) * 0.5f, R1_ZC = (R1_Z1 + R1_Z2) * 0.5f;
        float R1_W  = R1_X2 - R1_X1,           R1_D  = R1_Z2 - R1_Z1;
        // Floor
        m = glm::scale(glm::translate(glm::mat4(1.f), {R1_XC, FY - 0.06f, R1_ZC}), {R1_W, 0.10f, R1_D});
        drawCube(sh, m, glm::vec3(0.68f, 0.62f, 0.50f), texWood, 4.f);
        // Ceiling
        m = glm::scale(glm::translate(glm::mat4(1.f), {R1_XC, FY + CLG + 0.06f, R1_ZC}), {R1_W, 0.10f, R1_D});
        drawCube(sh, m, CREAM, texConcrete, 3.f);
        // South & west walls (interior)
        m = glm::scale(glm::translate(glm::mat4(1.f), {R1_XC, FY + CLG * 0.5f, R1_Z1 - WALL_T * 0.5f}), {R1_W + WALL_T, CLG, WALL_T});
        drawCube(sh, m, CREAM, texConcrete, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), {R1_X1 - WALL_T * 0.5f, FY + CLG * 0.5f, R1_ZC}), {WALL_T, CLG, R1_D});
        drawCube(sh, m, CREAM, texConcrete, 2.f);
        // Room divider (north) with door gap
        m = glm::scale(glm::translate(glm::mat4(1.f), {R1_X1 + (R1_W - 1.8f) * 0.25f, FY + CLG * 0.5f, R1_Z2 + WALL_T * 0.5f}), {(R1_W - 1.8f) * 0.5f, CLG, WALL_T});
        drawCube(sh, m, CREAM, texConcrete, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), {R1_X2 - (R1_W - 1.8f) * 0.25f, FY + CLG * 0.5f, R1_Z2 + WALL_T * 0.5f}), {(R1_W - 1.8f) * 0.5f, CLG, WALL_T});
        drawCube(sh, m, CREAM, texConcrete, 2.f);
        // Door
        m = glm::scale(glm::translate(glm::mat4(1.f), {R1_XC, FY + 1.20f, R1_Z2 + WALL_T * 0.5f}), {1.80f, 2.40f, WALL_T + 0.04f});
        drawCube(sh, m, WOOD_D, texWood, 1.f);
        // Pendant light
        pendantLight(sh, R1_XC, FY + CLG - 0.02f, R1_ZC);

        // FURNITURE – bed, desk, wardrobe, bookshelf, chair
        // Bed
        float bx = R1_X1 + 1.1f, bz = R1_Z1 + 2.8f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {bx, FY + 0.22f, bz}), {2.0f, 0.40f, 4.0f});
        drawCube(sh, m, WOOD_D, texWood, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), {bx, FY + 0.50f, bz}), {1.80f, 0.22f, 3.70f});
        drawCube(sh, m, glm::vec3(0.88f, 0.84f, 0.80f), texConcrete, 1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), {bx, FY + 0.64f, bz - 0.80f}), {1.78f, 0.14f, 2.10f});
        drawCube(sh, m, glm::vec3(0.22f, 0.36f, 0.62f));  // blue blanket
        m = glm::scale(glm::translate(glm::mat4(1.f), {bx, FY + 0.66f, bz + 1.45f}), {1.68f, 0.16f, 0.84f});
        drawCube(sh, m, glm::vec3(0.96f, 0.94f, 0.90f));  // pillow
        m = glm::scale(glm::translate(glm::mat4(1.f), {bx, FY + 1.05f, R1_Z1 + 0.30f}), {2.0f, 1.0f, 0.16f});
        drawCube(sh, m, WOOD_D, texWood, 1.f);  // headboard

        // Desk
        float dx = R1_X2 - 0.95f, dz = R1_Z1 + 2.5f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {dx, FY + 0.76f, dz}), {1.60f, 0.07f, 0.80f});
        drawCube(sh, m, WOOD_M, texWood, 1.f);
        for (int li : {-1, 1}) for (int lz2 : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), {dx + li * 0.68f, FY + 0.37f, dz + lz2 * 0.32f}), {0.09f, 0.74f, 0.09f});
            drawCylinder(sh, m, WOOD_M);
        }
        // Books on desk
        for (int b = 0; b < 3; b++) {
            glm::vec3 bc[3] = {{0.72f,0.18f,0.18f},{0.18f,0.42f,0.72f},{0.24f,0.58f,0.22f}};
            m = glm::scale(glm::translate(glm::mat4(1.f), {dx - 0.42f + b * 0.30f, FY + 0.90f, dz - 0.28f}), {0.10f, 0.30f, 0.24f});
            drawCube(sh, m, bc[b]);
        }

        // Chair
        float cz = dz + 0.70f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {dx, FY + 0.46f, cz}), {0.65f, 0.07f, 0.65f});
        drawCube(sh, m, WOOD_L, texWood, 1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), {dx, FY + 0.82f, cz - 0.32f}), {0.65f, 0.70f, 0.08f});
        drawCube(sh, m, WOOD_L, texWood, 1.f);
        for (int li : {-1, 1}) for (int lz2 : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), {dx + li * 0.26f, FY + 0.23f, cz + lz2 * 0.27f}), {0.07f, 0.46f, 0.07f});
            drawCylinder(sh, m, WOOD_L);
        }

        // Wardrobe
        float wx = R1_X2 - 0.88f, wz = R1_Z2 - 0.45f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {wx, FY + 1.25f, wz}), {1.50f, 2.50f, 0.72f});
        drawCube(sh, m, WOOD_L, texWood, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), {wx, FY + 2.52f, wz}), {1.62f, 0.12f, 0.80f});
        drawCube(sh, m, WOOD_M, texWood, 1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), {wx, FY + 1.68f, wz - 0.37f}), {0.58f, 1.22f, 0.04f});
        drawCrystalGlass(sh, m);  // mirror

        // Bookshelf on south wall
        float sx = R1_X1 + 1.4f, sz = R1_Z1 + 0.18f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {sx, FY + 1.88f, sz}), {2.40f, 1.30f, 0.26f});
        drawCube(sh, m, WOOD_M, texWood, 1.f);
        for (int sb : {0, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), {sx, FY + 1.25f + sb * 0.60f, sz + 0.11f}), {2.40f, 0.06f, 0.28f});
            drawCube(sh, m, WOOD_M, texWood, 1.f);
        }
        glm::vec3 bc[4] = {{0.72f,0.18f,0.18f},{0.18f,0.42f,0.72f},{0.24f,0.58f,0.22f},{0.70f,0.62f,0.16f}};
        for (int bk = 0; bk < 5; bk++) {
            float bkH = 0.26f + (bk % 3) * 0.05f;
            m = glm::scale(glm::translate(glm::mat4(1.f), {sx - 0.90f + bk * 0.40f, FY + 1.25f + bkH * 0.5f, sz + 0.13f}), {0.10f, bkH, 0.26f});
            drawCube(sh, m, bc[bk % 4]);
        }
    }

    // ── UPPER FLOOR INTERIOR – more corridor + 2nd room ──────────────────
    {
        const float FY = FH + 0.62f;
        const float CLG = FH - 0.38f;
        // Corridor ceiling pendants upper floor
        float CX1 = X1 - 2.5f, CXC = (CX1 + X1) * 0.5f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {CXC, FY + CLG + 0.06f, ZC}), {X1 - CX1, 0.10f, D});
        drawCube(sh, m, CREAM, texConcrete, 4.f);
        for (int i = 0; i < 5; i++) {
            float pz = Z0 + 3.5f + i * 6.0f;
            pendantLight(sh, CXC, FY + CLG - 0.02f, pz);
        }
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  RIGHT BUILDING  –  Common Facilities Wing
//  World: x = +10 .. +22,  z = -16 .. +16,  2 floors
// ════════════════════════════════════════════════════════════════════════════
void renderDormBlockB(unsigned int sh) {
    const float X0 = 10.f, X1 = 22.f, XC = (X0 + X1) * 0.5f;
    const float Z0 = -16.f, Z1 = 16.f, ZC = (Z0 + Z1) * 0.5f;
    const float W  = X1 - X0;
    const float D  = Z1 - Z0;
    const float FH = 3.6f;
    const float TH = FH * 2.f;
    const float WALL_T = 0.35f;

    glm::mat4 m;

    // ── EXTERIOR SHELL (mirror of left building) ──────────────────────────

    // Brick lower
    m = glm::scale(glm::translate(glm::mat4(1.f), {XC, FH * 0.25f, ZC}), {W, FH * 0.5f, D});
    drawCube(sh, m, BRICK, texBrick, 8.f);
    // Concrete upper
    m = glm::scale(glm::translate(glm::mat4(1.f), {XC, FH + FH * 0.5f, ZC}), {W, FH, D});
    drawCube(sh, m, CONC, texConcrete, 8.f);
    // Floor slab
    m = glm::scale(glm::translate(glm::mat4(1.f), {XC, FH, ZC}), {W + 0.1f, 0.28f, D + 0.1f});
    drawCube(sh, m, TRIM, texConcrete, 5.f);
    // Roof
    m = glm::scale(glm::translate(glm::mat4(1.f), {XC, TH + 0.18f, ZC}), {W + 0.8f, 0.38f, D + 0.8f});
    drawCube(sh, m, ROOF_C, texConcrete, 6.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), {XC, TH + 0.46f, ZC}), {W + 0.82f, 0.35f, D + 0.82f});
    drawCube(sh, m, TRIM, texConcrete, 6.f);

    // LEFT (courtyard-facing) windows – both floors
    for (int fl = 0; fl < 2; fl++) {
        float fy = fl * FH;
        for (int i = 0; i < 4; i++) {
            float wz = Z0 + 4.f + i * 6.8f;
            m = glm::scale(glm::translate(glm::mat4(1.f), {X0 + 0.01f, fy + FH * 0.56f, wz}), {WALL_T + 0.04f, 1.80f, 2.60f});
            drawCrystalGlass(sh, m);
            m = glm::scale(glm::translate(glm::mat4(1.f), {X0 + 0.01f, fy + FH * 0.56f + 0.98f, wz}), {WALL_T + 0.06f, 0.22f, 2.80f});
            drawCube(sh, m, TRIM, texConcrete, 1.f);
            m = glm::scale(glm::translate(glm::mat4(1.f), {X0 + 0.01f, fy + FH * 0.56f - 1.00f, wz}), {WALL_T + 0.06f, 0.14f, 2.82f});
            drawCube(sh, m, CONC, texConcrete, 1.f);
        }
        for (int i = 0; i < 3; i++) {
            float wz = Z0 + 7.5f + i * 6.8f;
            wallLight(sh, X0 - 0.04f, fy + FH * 0.72f, wz, -1);
        }
    }

    // Right outer face – windows
    for (int fl = 0; fl < 2; fl++) {
        float fy = fl * FH;
        for (int i = 0; i < 3; i++) {
            float wz = Z0 + 5.f + i * 9.0f;
            m = glm::scale(glm::translate(glm::mat4(1.f), {X1 - 0.01f, fy + FH * 0.56f, wz}), {WALL_T + 0.04f, 1.60f, 2.20f});
            drawGlass(sh, m);
        }
    }
    drainPipe(sh, X1 + 0.04f, Z0 + 2.f,  TH + 0.5f);
    drainPipe(sh, X1 + 0.04f, Z0 + 16.f, TH + 0.5f);

    // ── GROUND FLOOR: COMMON ROOM (left half) + READING ROOM (right half) ─
    {
        const float FY = 0.62f;
        const float CLG = FH - 0.38f;

        // Common room: x = X0..X0+6, z full
        float CRX0 = X0 + WALL_T, CRX1 = X0 + 6.0f;
        float CRXC = (CRX0 + CRX1) * 0.5f;
        // Floor
        m = glm::scale(glm::translate(glm::mat4(1.f), {CRXC, FY - 0.06f, ZC}), {CRX1 - CRX0, 0.10f, D});
        drawCube(sh, m, FLOOR_C, texTile, 8.f);
        // Ceiling + pendant
        m = glm::scale(glm::translate(glm::mat4(1.f), {CRXC, FY + CLG + 0.06f, ZC}), {CRX1 - CRX0, 0.10f, D});
        drawCube(sh, m, CREAM, texConcrete, 4.f);
        pendantLight(sh, CRXC, FY + CLG - 0.02f, ZC - 4.f);
        pendantLight(sh, CRXC, FY + CLG - 0.02f, ZC + 4.f);

        // SOFA set (3 sofas around a coffee table)
        // Main sofa (facing right)
        float sx = CRX0 + 0.55f, sz = ZC - 2.0f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {sx, FY + 0.46f, sz}), {0.80f, 0.46f, 2.60f});
        drawCube(sh, m, SOFA_C);
        m = glm::scale(glm::translate(glm::mat4(1.f), {sx - 0.38f, FY + 0.72f, sz}), {0.10f, 0.50f, 2.60f});
        drawCube(sh, m, SOFA_C);  // backrest
        // Side sofa
        sz = ZC + 2.4f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {CRXC + 0.40f, FY + 0.46f, sz}), {2.0f, 0.46f, 0.80f});
        drawCube(sh, m, SOFA_C);
        m = glm::scale(glm::translate(glm::mat4(1.f), {CRXC + 0.40f, FY + 0.72f, sz + 0.38f}), {2.0f, 0.50f, 0.10f});
        drawCube(sh, m, SOFA_C);
        // Coffee table
        m = glm::scale(glm::translate(glm::mat4(1.f), {CRXC + 0.20f, FY + 0.34f, ZC}), {1.20f, 0.08f, 1.20f});
        drawCube(sh, m, WOOD_M, texWood, 1.f);
        for (int li : {-1, 1}) for (int lz2 : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), {CRXC + 0.20f + li * 0.48f, FY + 0.17f, ZC + lz2 * 0.48f}), {0.08f, 0.34f, 0.08f});
            drawCylinder(sh, m, WOOD_D);
        }
        // TV on right wall of common room (facing left)
        {
            float tvX = CRX1 - 0.14f;
            float tvY = FY + 1.60f;
            float tvZ = ZC - 5.0f;
            float TWW = 2.60f, TWH = 1.50f;
            m = glm::scale(glm::translate(glm::mat4(1.f), {tvX, tvY, tvZ}), {0.22f, TWH + 0.18f, TWW + 0.18f});
            drawCube(sh, m, glm::vec3(0.08f, 0.08f, 0.09f));
            float t2 = (float)glfwGetTime();
            float bv = 0.25f + 0.10f * sinf(t2 * 0.5f);
            glm::vec3 scrC = {bv * 0.20f, bv * 0.45f, bv * 0.95f};
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(scrC * 0.70f));
            m = glm::scale(glm::translate(glm::mat4(1.f), {tvX - 0.02f, tvY, tvZ}), {0.06f, TWH, TWW});
            drawCube(sh, m, scrC);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
        }
        // Indoor plant in corner
        {
            float px = CRX0 + 0.40f, pz2 = Z1 - WALL_T - 0.50f;
            m = glm::scale(glm::translate(glm::mat4(1.f), {px, FY + 0.22f, pz2}), {0.50f, 0.44f, 0.50f});
            drawCylinder(sh, m, BRICK, texBrick, 1.f);  // pot
            m = glm::scale(glm::translate(glm::mat4(1.f), {px, FY + 0.55f, pz2}), {0.08f, 0.55f, 0.08f});
            drawCylinder(sh, m, COL_TRUNK);
            m = glm::scale(glm::translate(glm::mat4(1.f), {px, FY + 1.10f, pz2}), {0.80f, 0.80f, 0.80f});
            drawSphere(sh, m, PLANT, texGrass, 1.f);
        }

        // Reading room: x = X0+6 .. X1
        float RRX0 = CRX1 + WALL_T, RRX1 = X1 - WALL_T;
        float RRXC = (RRX0 + RRX1) * 0.5f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {RRXC, FY - 0.06f, ZC}), {RRX1 - RRX0, 0.10f, D});
        drawCube(sh, m, glm::vec3(0.74f, 0.68f, 0.58f), texWood, 6.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), {RRXC, FY + CLG + 0.06f, ZC}), {RRX1 - RRX0, 0.10f, D});
        drawCube(sh, m, CREAM, texConcrete, 4.f);
        // Divider wall
        m = glm::scale(glm::translate(glm::mat4(1.f), {CRX1, FY + CLG * 0.5f, ZC}), {WALL_T, CLG, D});
        drawCube(sh, m, CREAM, texConcrete, 3.f);
        // Pendant lights
        pendantLight(sh, RRXC, FY + CLG - 0.02f, ZC - 5.f);
        pendantLight(sh, RRXC, FY + CLG - 0.02f, ZC + 5.f);
        // Bookshelf on outer wall
        for (int s = 0; s < 2; s++) {
            float bsx = X1 - 0.20f, bsz = Z0 + 4.0f + s * 8.0f;
            m = glm::scale(glm::translate(glm::mat4(1.f), {bsx, FY + 1.80f, bsz}), {0.26f, 1.50f, 3.20f});
            drawCube(sh, m, WOOD_M, texWood, 1.f);
            for (int sb : {0, 1}) {
                m = glm::scale(glm::translate(glm::mat4(1.f), {bsx - 0.11f, FY + 1.22f + sb * 0.65f, bsz}), {0.28f, 0.06f, 3.20f});
                drawCube(sh, m, WOOD_M, texWood, 1.f);
            }
            glm::vec3 bkc[4] = {{0.72f,0.18f,0.18f},{0.18f,0.42f,0.72f},{0.24f,0.58f,0.22f},{0.70f,0.62f,0.16f}};
            for (int bk = 0; bk < 6; bk++) {
                float bkH = 0.28f + (bk % 3) * 0.05f;
                m = glm::scale(glm::translate(glm::mat4(1.f), {bsx - 0.13f, FY + 1.22f + bkH * 0.5f, bsz - 1.40f + bk * 0.50f}), {0.28f, bkH, 0.12f});
                drawCube(sh, m, bkc[bk % 4]);
            }
        }
        // Reading desks (2 rows of 3)
        for (int row = 0; row < 2; row++) {
            float dz2 = ZC - 7.5f + row * 6.5f;
            for (int col = 0; col < 2; col++) {
                float dx2 = RRXC - 1.4f + col * 2.8f;
                m = glm::scale(glm::translate(glm::mat4(1.f), {dx2, FY + 0.75f, dz2}), {1.70f, 0.07f, 0.80f});
                drawCube(sh, m, WOOD_L, texWood, 1.f);
                for (int li : {-1, 1}) {
                    m = glm::scale(glm::translate(glm::mat4(1.f), {dx2 + li * 0.72f, FY + 0.36f, dz2}), {0.08f, 0.72f, 0.08f});
                    drawCylinder(sh, m, WOOD_D);
                }
                // Desk lamp
                m = glm::scale(glm::translate(glm::mat4(1.f), {dx2 + 0.60f, FY + 0.88f, dz2 - 0.28f}), {0.05f, 0.24f, 0.05f});
                drawCylinder(sh, m, glm::vec3(0.50f, 0.50f, 0.52f));
                {
                    glm::mat4 sh2 = glm::translate(glm::mat4(1.f), {dx2 + 0.60f, FY + 1.05f, dz2 - 0.28f});
                    sh2 = glm::rotate(sh2, glm::radians(180.f), {1.f, 0.f, 0.f});
                    sh2 = glm::scale(sh2, {0.22f, 0.18f, 0.22f});
                    drawCone(sh, sh2, glm::vec3(0.65f, 0.60f, 0.45f));
                }
                // Chair
                m = glm::scale(glm::translate(glm::mat4(1.f), {dx2, FY + 0.46f, dz2 + 0.68f}), {0.62f, 0.07f, 0.62f});
                drawCube(sh, m, WOOD_L, texWood, 1.f);
                m = glm::scale(glm::translate(glm::mat4(1.f), {dx2, FY + 0.80f, dz2 + 0.36f}), {0.62f, 0.66f, 0.08f});
                drawCube(sh, m, WOOD_L, texWood, 1.f);
            }
        }
    }

    // ── UPPER FLOOR: DINING HALL ──────────────────────────────────────────
    {
        const float FY = FH + 0.62f;
        const float CLG = FH - 0.38f;

        // Floor
        m = glm::scale(glm::translate(glm::mat4(1.f), {XC, FY - 0.06f, ZC}), {W, 0.10f, D});
        drawCube(sh, m, glm::vec3(0.78f, 0.73f, 0.62f), texTile, 8.f);
        // Ceiling + pendants
        m = glm::scale(glm::translate(glm::mat4(1.f), {XC, FY + CLG + 0.06f, ZC}), {W, 0.10f, D});
        drawCube(sh, m, CREAM, texConcrete, 5.f);
        for (int i = 0; i < 4; i++) {
            float pz = Z0 + 4.0f + i * 7.5f;
            pendantLight(sh, XC - 1.5f, FY + CLG - 0.02f, pz);
            pendantLight(sh, XC + 1.5f, FY + CLG - 0.02f, pz);
        }

        // Dining tables (3 rows x 2 cols)
        for (int row = 0; row < 3; row++) {
            float tz = Z0 + 4.5f + row * 9.0f;
            for (int col = 0; col < 2; col++) {
                float tx = XC - 2.8f + col * 5.6f;
                // Table top
                m = glm::scale(glm::translate(glm::mat4(1.f), {tx, FY + 0.76f, tz}), {2.40f, 0.08f, 1.20f});
                drawCube(sh, m, WOOD_L, texWood, 1.f);
                // Table legs
                for (int li : {-1, 1}) for (int lz2 : {-1, 1}) {
                    m = glm::scale(glm::translate(glm::mat4(1.f), {tx + li * 1.02f, FY + 0.37f, tz + lz2 * 0.46f}), {0.09f, 0.74f, 0.09f});
                    drawCylinder(sh, m, WOOD_D);
                }
                // Chairs around table
                for (int side : {-1, 1}) {
                    for (int ci = 0; ci < 2; ci++) {
                        float cz2 = tz + (ci == 0 ? -0.90f : 0.90f);
                        m = glm::scale(glm::translate(glm::mat4(1.f), {tx + side * 0.0f, FY + 0.45f, cz2}), {0.60f, 0.07f, 0.60f});
                        drawCube(sh, m, WOOD_L, texWood, 1.f);
                        m = glm::scale(glm::translate(glm::mat4(1.f), {tx, FY + 0.78f, cz2 + side * 0.30f}), {0.60f, 0.62f, 0.08f});
                        drawCube(sh, m, WOOD_L, texWood, 1.f);
                    }
                }
            }
        }

        // Kitchen counter at north end (z = Z1 side)
        float kz = Z1 - 0.60f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {XC, FY + 0.90f, kz}), {W - 1.0f, 0.10f, 0.80f});
        drawCube(sh, m, CONC, texConcrete, 3.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), {XC, FY + 0.50f, kz}), {W - 1.0f, 0.80f, 0.78f});
        drawCube(sh, m, CREAM, texConcrete, 3.f);
        // Overhead cabinet
        m = glm::scale(glm::translate(glm::mat4(1.f), {XC, FY + CLG - 0.40f, kz + 0.30f}), {W - 1.2f, 0.60f, 0.45f});
        drawCube(sh, m, WOOD_L, texWood, 2.f);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  CONNECTING CORRIDOR  – now the central covered walkway along the courtyard
//  (kept in API but rendered as the front gateway canopy / entrance tunnel)
// ════════════════════════════════════════════════════════════════════════════
void renderCorridor(unsigned int sh) {
    // Thin covered walkway strips along both buildings' courtyard facades
    const float CANY = 3.60f;  // canopy height
    const float CAND = 1.60f;  // canopy depth
    const float Z0 = -16.f, Z1 = 16.f;

    glm::mat4 m;

    // Left building walkway (x = -10 .. -10+1.6)
    m = glm::scale(glm::translate(glm::mat4(1.f), {-10.f + CAND * 0.5f, CANY + 0.14f, 0.f}), {CAND, 0.28f, Z1 - Z0});
    drawCube(sh, m, ROOF_C, texConcrete, 5.f);
    // Support columns for left walkway
    for (int i = 0; i < 6; i++) {
        float cz = Z0 + 2.5f + i * 5.5f;
        glm::mat4 cy = glm::scale(glm::translate(glm::mat4(1.f), {-10.f + CAND, CANY * 0.5f, cz}), {0.28f, CANY, 0.28f});
        drawCylinder(sh, cy, CONC, texConcrete, 1.f);
    }

    // Right building walkway (x = 10-1.6 .. 10)
    m = glm::scale(glm::translate(glm::mat4(1.f), {10.f - CAND * 0.5f, CANY + 0.14f, 0.f}), {CAND, 0.28f, Z1 - Z0});
    drawCube(sh, m, ROOF_C, texConcrete, 5.f);
    for (int i = 0; i < 6; i++) {
        float cz = Z0 + 2.5f + i * 5.5f;
        glm::mat4 cy = glm::scale(glm::translate(glm::mat4(1.f), {10.f - CAND, CANY * 0.5f, cz}), {0.28f, CANY, 0.28f});
        drawCylinder(sh, cy, CONC, texConcrete, 1.f);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  COURTYARD  –  central grass area with benches, trees, lamps, staircase
// ════════════════════════════════════════════════════════════════════════════
void renderCourtyard(unsigned int sh) {
    glm::mat4 m;

    // Grass bed (between the two buildings)
    m = glm::scale(glm::translate(glm::mat4(1.f), {0.f, 0.01f, 0.f}), {19.0f, 0.08f, 31.0f});
    drawCube(sh, m, COL_GRASS, texGrass, 6.f);

    // Central paved pathway (z from +28 down to -14)
    m = glm::scale(glm::translate(glm::mat4(1.f), {0.f, 0.04f, 7.0f}), {4.0f, 0.06f, 42.0f});
    drawCube(sh, m, COL_PLAZA, texTile, 8.f);

    // Decorative round trees (in planters, 4 per side)
    for (int s : {-1, 1}) {
        for (int i = 0; i < 4; i++) {
            float tz = -10.f + i * 6.5f;
            decorTree(sh, s * 4.5f, tz);
        }
    }

    // Courtyard benches (4 pairs around the central path)
    auto bench = [&](float x, float z, float rotY) {
        glm::mat4 base = glm::rotate(glm::translate(glm::mat4(1.f), {x, 0.72f, z}), glm::radians(rotY), {0,1,0});
        drawCube(sh, glm::scale(glm::translate(base, {0.f, 0.f, 0.f}), {2.0f, 0.11f, 0.50f}), WOOD_L, texWood, 1.f);
        for (int s : {-1, 1}) {
            drawCube(sh, glm::scale(glm::translate(base, {s*0.80f, -0.30f, 0.18f}), {0.10f, 0.60f, 0.10f}), TRIM);
            drawCube(sh, glm::scale(glm::translate(base, {s*0.80f, -0.30f, -0.18f}), {0.10f, 0.60f, 0.10f}), TRIM);
        }
        drawCube(sh, glm::scale(glm::translate(base, {0.f, 0.38f, -0.22f}), {2.0f, 0.44f, 0.08f}), WOOD_L, texWood, 1.f);
    };
    for (int i = 0; i < 3; i++) {
        float bz = -8.0f + i * 6.5f;
        bench(-3.0f, bz,  15.f);
        bench( 3.0f, bz, -15.f);
    }

    // ── CENTRAL STAIRCASE at back of courtyard (z = -12 .. -4) ──────────
    // Wide staircase going UP to connect to elevated walkway / upper floor level
    // 8 steps, 0.22 rise each, 0.60 run each
    const float SX = 0.f, SZ0 = -14.0f;
    const float SWIDTH = 8.0f;
    const float RISE = 0.22f, RUN = 0.70f;
    for (int step = 0; step < 8; step++) {
        float sy = (step + 1) * RISE;
        float sz = SZ0 + step * RUN;
        m = glm::scale(glm::translate(glm::mat4(1.f), {SX, sy * 0.5f, sz}), {SWIDTH, sy, RUN});
        drawCube(sh, m, CONC, texConcrete, 2.f);
    }
    // Staircase back wall / retaining
    m = glm::scale(glm::translate(glm::mat4(1.f), {SX, 1.0f, SZ0 - 0.35f}), {SWIDTH, 2.0f, 0.35f});
    drawCube(sh, m, BRICK, texBrick, 2.f);
    // Stair railings (both sides, metal)
    for (int s : {-1, 1}) {
        float rx = SX + s * (SWIDTH * 0.5f - 0.10f);
        // Bottom post
        m = glm::scale(glm::translate(glm::mat4(1.f), {rx, 0.50f, SZ0}), {0.08f, 1.0f, 0.08f});
        drawCylinder(sh, m, IRON);
        // Top post
        float topZ = SZ0 + 7 * RUN;
        float topY = 8 * RISE + 0.50f;
        m = glm::scale(glm::translate(glm::mat4(1.f), {rx, topY * 0.5f, topZ}), {0.08f, topY, 0.08f});
        drawCylinder(sh, m, IRON);
        // Handrail bar (diagonal – approximated as tilted cube)
        glm::mat4 rail = glm::translate(glm::mat4(1.f), {rx, (topY + 0.50f) * 0.5f, (SZ0 + topZ) * 0.5f});
        float ang = atanf((8 * RISE) / (7 * RUN));
        rail = glm::rotate(rail, ang, {1.f, 0.f, 0.f});
        float rLen = sqrtf((7 * RUN) * (7 * RUN) + (8 * RISE - 0.50f) * (8 * RISE - 0.50f));
        rail = glm::scale(rail, {0.08f, 0.08f, rLen});
        drawCube(sh, rail, IRON);
    }
    // Elevated landing at top of stairs
    m = glm::scale(glm::translate(glm::mat4(1.f), {SX, 8 * RISE + 0.08f, SZ0 + 7 * RUN + 1.0f}), {SWIDTH, 0.16f, 2.5f});
    drawCube(sh, m, CONC, texConcrete, 2.f);
}

// ════════════════════════════════════════════════════════════════════════════
//  ADMIN ANNEX  –  repurposed as small utility/storage block on right side
// ════════════════════════════════════════════════════════════════════════════
void renderAdminAnnex(unsigned int sh) {
    // Small single-storey block tucked behind right building
    const float AX = 20.f, AZ = -20.f;
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), {AX, 1.80f, AZ}), {5.0f, 3.60f, 5.0f});
    drawCube(sh, m, BRICK, texBrick, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), {AX, 3.72f, AZ}), {5.2f, 0.28f, 5.2f});
    drawCube(sh, m, ROOF_C, texConcrete, 2.f);
    // Door
    m = glm::scale(glm::translate(glm::mat4(1.f), {AX, 1.20f, AZ + 2.52f}), {1.50f, 2.40f, 0.22f});
    drawCube(sh, m, WOOD_D, texWood, 1.f);
}

// ════════════════════════════════════════════════════════════════════════════
//  ENTRANCE SPHERE  – small decorative pillar sphere at gate
// ════════════════════════════════════════════════════════════════════════════
void renderEntranceSphere(unsigned int sh) {
    // Gate pillar lights (small spheres on top of gate pillars)
    for (int s : {-1, 1}) {
        float px = -5.f + s * 5.2f;
        glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), {px, 4.80f, 28.f}), {0.55f, 0.55f, 0.55f});
        drawSphere(sh, m, glm::vec3(0.90f, 0.82f, 0.62f), texMarble, 1.f);
        if (!dayMode) {
            glm::vec3 em(0.90f, 0.80f, 0.40f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(em * 0.70f));
            m = glm::scale(glm::translate(glm::mat4(1.f), {px, 4.80f, 28.f}), {0.55f, 0.55f, 0.55f});
            drawSphere(sh, m, em, texMarble, 1.f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
        }
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  ROOM INTERIOR  –  same as before (inside Block A, O key teleport target)
// ════════════════════════════════════════════════════════════════════════════
void renderRoomInterior(unsigned int sh) {
    // Rooms are now inside the left building – rendered as part of renderDormBlockA
    // This function is kept for API compatibility; left empty to avoid double-draw
}

// ════════════════════════════════════════════════════════════════════════════
//  IMMERSIVE DORM ROOM  (O key)  –  standalone twin-bed room
// ════════════════════════════════════════════════════════════════════════════
void renderDormRoom(unsigned int sh) {
    static const glm::vec3 ZERO3(0.f, 0.f, 0.f);

    const float RX = 200.f, RZ = 200.f;
    const float RW = 12.f, RD = 9.f, RH = 3.20f;
    const float WALL_T = 0.28f;
    const float FY = 0.62f;

    const glm::vec3 WALL_C (0.94f, 0.91f, 0.84f);
    const glm::vec3 FLOOR_CLR(0.55f, 0.38f, 0.18f);
    const glm::vec3 CLG_C  (0.99f, 0.98f, 0.96f);
    const glm::vec3 ACCENT (0.20f, 0.55f, 0.52f);
    const glm::vec3 SKIRTING(0.38f, 0.26f, 0.10f);
    const glm::vec3 BED_F  (0.30f, 0.18f, 0.08f);
    const glm::vec3 MATT   (0.90f, 0.86f, 0.80f);
    const glm::vec3 PILLOW2(0.97f, 0.95f, 0.92f);
    const glm::vec3 BLANK1 (0.22f, 0.38f, 0.62f);
    const glm::vec3 BLANK2 (0.55f, 0.20f, 0.20f);
    const glm::vec3 TAB_C  (0.50f, 0.34f, 0.16f);
    const glm::vec3 CH_C   (0.26f, 0.16f, 0.06f);
    const glm::vec3 SH_C   (0.42f, 0.28f, 0.12f);
    const glm::vec3 WARD_C (0.40f, 0.28f, 0.12f);
    const glm::vec3 BOOK[4]= {
        {0.72f,0.18f,0.18f},{0.18f,0.40f,0.72f},
        {0.22f,0.58f,0.25f},{0.74f,0.66f,0.18f}
    };

    glm::mat4 m;

    // Shell
    m = glm::scale(glm::translate(glm::mat4(1.f), {RX, FY-0.07f, RZ}), {RW, 0.14f, RD});
    drawCube(sh, m, FLOOR_CLR, texWood, 6.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), {RX, FY+RH+0.07f, RZ}), {RW+0.6f, 0.14f, RD+0.6f});
    drawCube(sh, m, CLG_C, texConcrete, 4.f);
    // South wall (windows)
    m = glm::scale(glm::translate(glm::mat4(1.f), {RX, FY+RH*0.5f, RZ-RD*0.5f-WALL_T*0.5f}), {RW+WALL_T*2, RH, WALL_T});
    drawCube(sh, m, WALL_C, texConcrete, 3.f);
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), {RX+s*2.5f, FY+RH*0.58f, RZ-RD*0.5f-WALL_T*0.5f}), {2.8f, 1.8f, WALL_T+0.04f});
        drawCrystalGlass(sh, m);
        m = glm::scale(glm::translate(glm::mat4(1.f), {RX+s*2.5f, FY+RH*0.58f+0.98f, RZ-RD*0.5f-WALL_T*0.5f}), {3.0f, 0.22f, WALL_T+0.06f});
        drawCube(sh, m, ACCENT);
        m = glm::scale(glm::translate(glm::mat4(1.f), {RX+s*2.5f, FY+RH*0.58f-1.0f, RZ-RD*0.5f}), {3.1f, 0.12f, 0.30f});
        drawCube(sh, m, CREAM, texConcrete, 1.f);
    }
    // North wall (door)
    float doorW=1.80f, doorH=2.40f;
    m = glm::scale(glm::translate(glm::mat4(1.f), {RX-(RW-doorW)*0.25f, FY+RH*0.5f, RZ+RD*0.5f+WALL_T*0.5f}), {(RW-doorW)*0.5f, RH, WALL_T});
    drawCube(sh, m, WALL_C, texConcrete, 3.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), {RX+(RW-doorW)*0.25f, FY+RH*0.5f, RZ+RD*0.5f+WALL_T*0.5f}), {(RW-doorW)*0.5f, RH, WALL_T});
    drawCube(sh, m, WALL_C, texConcrete, 3.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), {RX, FY+doorH+(RH-doorH)*0.5f, RZ+RD*0.5f+WALL_T*0.5f}), {doorW, RH-doorH, WALL_T});
    drawCube(sh, m, WALL_C, texConcrete, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), {RX, FY+doorH*0.5f, RZ+RD*0.5f+WALL_T*0.5f}), {doorW, doorH, WALL_T+0.04f});
    drawCube(sh, m, WARD_C, texWood, 1.f);
    // East & West walls with accent stripe
    for (int s : {-1,1}) {
        float wx = RX + s*(RW*0.5f+WALL_T*0.5f);
        m = glm::scale(glm::translate(glm::mat4(1.f), {wx, FY+RH*0.5f, RZ}), {WALL_T, RH, RD});
        drawCube(sh, m, WALL_C, texConcrete, 3.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), {wx, FY+RH*0.62f, RZ}), {WALL_T+0.01f, 0.18f, RD});
        drawCube(sh, m, ACCENT);
    }
    // Skirting
    {
        float sy = FY+0.09f;
        m = glm::scale(glm::translate(glm::mat4(1.f),{RX,sy,RZ-RD*0.5f}),{RW,0.18f,0.05f}); drawCube(sh,m,SKIRTING,texWood,1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f),{RX,sy,RZ+RD*0.5f}),{RW,0.18f,0.05f}); drawCube(sh,m,SKIRTING,texWood,1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f),{RX-RW*0.5f,sy,RZ}),{0.05f,0.18f,RD}); drawCube(sh,m,SKIRTING,texWood,1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f),{RX+RW*0.5f,sy,RZ}),{0.05f,0.18f,RD}); drawCube(sh,m,SKIRTING,texWood,1.f);
    }

    // Ceiling lights + fan
    float lampOffsets[2] = { -2.5f, 2.5f };
    for (int i = 0; i < 2; i++) {
        pendantLight(sh, RX + lampOffsets[i], FY+RH-0.04f, RZ);
    }
    // Fan
    {
        float fX=RX, fY2=FY+RH-0.10f, fZ=RZ;
        float t = (float)glfwGetTime();
        m = glm::scale(glm::translate(glm::mat4(1.f),{fX,fY2,fZ}),{0.30f,0.18f,0.30f});
        drawCylinder(sh, m, glm::vec3(0.55f,0.55f,0.58f));
        for (int b = 0; b < 4; b++) {
            float ang = t*120.f + b*90.f;
            glm::mat4 blade = glm::translate(glm::mat4(1.f),{fX,fY2-0.05f,fZ});
            blade = glm::rotate(blade, glm::radians(ang),{0.f,1.f,0.f});
            blade = glm::translate(blade,{1.1f,0.f,0.f});
            blade = glm::scale(blade,{1.60f,0.06f,0.42f});
            drawCube(sh, blade, glm::vec3(0.70f,0.64f,0.52f), texWood, 1.f);
        }
        m = glm::scale(glm::translate(glm::mat4(1.f),{fX,fY2+0.12f,fZ}),{0.06f,0.20f,0.06f});
        drawCylinder(sh, m, glm::vec3(0.50f,0.50f,0.52f));
    }

    // BED 1 (left, blue blanket)
    { float bx=RX-3.8f, bz=RZ;
      m = glm::scale(glm::translate(glm::mat4(1.f),{bx,FY+0.22f,bz}),{2.20f,0.44f,4.20f}); drawCube(sh,m,BED_F,texWood,2.f);
      for (int lz:{-1,1}) for (int lx2:{-1,1}) { m = glm::scale(glm::translate(glm::mat4(1.f),{bx+lx2*0.90f,FY+0.11f,bz+lz*1.8f}),{0.18f,0.22f,0.18f}); drawCylinder(sh,m,BED_F); }
      m = glm::scale(glm::translate(glm::mat4(1.f),{bx,FY+0.52f,bz}),{2.00f,0.24f,3.90f}); drawCube(sh,m,MATT,texConcrete,1.f);
      m = glm::scale(glm::translate(glm::mat4(1.f),{bx,FY+0.66f,bz-0.80f}),{2.00f,0.16f,2.20f}); drawCube(sh,m,BLANK1);
      m = glm::scale(glm::translate(glm::mat4(1.f),{bx,FY+0.68f,bz+1.55f}),{1.80f,0.18f,0.90f}); drawCube(sh,m,PILLOW2);
      m = glm::scale(glm::translate(glm::mat4(1.f),{bx,FY+1.10f,RZ-RD*0.5f+0.40f}),{2.20f,1.10f,0.18f}); drawCube(sh,m,BED_F,texWood,1.f);
    }
    // BED 2 (right, red blanket)
    { float bx=RX+3.8f, bz=RZ;
      m = glm::scale(glm::translate(glm::mat4(1.f),{bx,FY+0.22f,bz}),{2.20f,0.44f,4.20f}); drawCube(sh,m,BED_F,texWood,2.f);
      for (int lz:{-1,1}) for (int lx2:{-1,1}) { m = glm::scale(glm::translate(glm::mat4(1.f),{bx+lx2*0.90f,FY+0.11f,bz+lz*1.8f}),{0.18f,0.22f,0.18f}); drawCylinder(sh,m,BED_F); }
      m = glm::scale(glm::translate(glm::mat4(1.f),{bx,FY+0.52f,bz}),{2.00f,0.24f,3.90f}); drawCube(sh,m,MATT,texConcrete,1.f);
      m = glm::scale(glm::translate(glm::mat4(1.f),{bx,FY+0.66f,bz-0.80f}),{2.00f,0.16f,2.20f}); drawCube(sh,m,BLANK2);
      m = glm::scale(glm::translate(glm::mat4(1.f),{bx,FY+0.68f,bz+1.55f}),{1.80f,0.18f,0.90f}); drawCube(sh,m,PILLOW2);
      m = glm::scale(glm::translate(glm::mat4(1.f),{bx,FY+1.10f,RZ-RD*0.5f+0.40f}),{2.20f,1.10f,0.18f}); drawCube(sh,m,BED_F,texWood,1.f);
    }
    // 2 study tables + chairs
    for (int s : {-1,1}) {
        float tx=RX+s*4.8f, tz=RZ+RD*0.5f-2.2f;
        m = glm::scale(glm::translate(glm::mat4(1.f),{tx,FY+0.75f,tz}),{1.80f,0.08f,0.90f}); drawCube(sh,m,TAB_C,texWood,1.f);
        for (int li:{-1,1}) for (int lz2:{-1,1}) { m=glm::scale(glm::translate(glm::mat4(1.f),{tx+li*0.78f,FY+0.37f,tz+lz2*0.35f}),{0.10f,0.74f,0.10f}); drawCylinder(sh,m,TAB_C); }
        float cz=tz+0.80f;
        m=glm::scale(glm::translate(glm::mat4(1.f),{tx,FY+0.48f,cz}),{0.70f,0.08f,0.70f}); drawCube(sh,m,CH_C,texWood,1.f);
        m=glm::scale(glm::translate(glm::mat4(1.f),{tx,FY+0.85f,cz-0.34f}),{0.70f,0.75f,0.08f}); drawCube(sh,m,CH_C,texWood,1.f);
        for (int li:{-1,1}) for (int lz2:{-1,1}) { m=glm::scale(glm::translate(glm::mat4(1.f),{tx+li*0.28f,FY+0.24f,cz+lz2*0.28f}),{0.08f,0.48f,0.08f}); drawCylinder(sh,m,CH_C); }
    }
    // 2 wardrobes
    for (int s : {-1,1}) {
        float wx=RX+s*4.8f, wz=RZ-RD*0.5f+0.42f;
        m=glm::scale(glm::translate(glm::mat4(1.f),{wx,FY+1.25f,wz}),{1.50f,2.50f,0.74f}); drawCube(sh,m,WARD_C,texWood,2.f);
        m=glm::scale(glm::translate(glm::mat4(1.f),{wx,FY+1.70f,wz-0.39f}),{0.60f,1.30f,0.04f}); drawCrystalGlass(sh,m);
        for (int h:{-1,1}) { m=glm::scale(glm::translate(glm::mat4(1.f),{wx+h*0.32f,FY+1.30f,wz-0.39f}),{0.06f,0.24f,0.06f}); drawCylinder(sh,m,glm::vec3(0.75f,0.68f,0.45f)); }
    }
    // Bookshelf
    { float sx=RX, sz=RZ-RD*0.5f+0.16f;
      m=glm::scale(glm::translate(glm::mat4(1.f),{sx,FY+1.90f,sz}),{4.0f,1.60f,0.30f}); drawCube(sh,m,SH_C,texWood,1.f);
      for (int sb:{0,1}) { m=glm::scale(glm::translate(glm::mat4(1.f),{sx,FY+1.20f+sb*0.72f,sz+0.13f}),{4.0f,0.07f,0.34f}); drawCube(sh,m,SH_C,texWood,1.f); }
      for (int bk=0;bk<9;bk++) { float bkH=0.28f+(bk%3)*0.06f; int row=(bk<5)?0:1; float bkY=FY+1.20f+row*0.72f+bkH*0.5f; m=glm::scale(glm::translate(glm::mat4(1.f),{sx-1.8f+bk*0.42f,bkY,sz+0.15f}),{0.10f,bkH,0.30f}); drawCube(sh,m,BOOK[bk%4]); }
    }
    // Notice board
    { float nx=RX+4.0f, ny=FY+1.60f, nz=RZ+RD*0.5f+WALL_T*0.5f;
      m=glm::scale(glm::translate(glm::mat4(1.f),{nx,ny,nz}),{0.10f,1.12f,1.68f}); drawCube(sh,m,SH_C);
      for (int p=0;p<3;p++) { m=glm::scale(glm::translate(glm::mat4(1.f),{nx,ny-0.25f+p*0.28f,nz-0.50f+p*0.44f}),{0.07f,0.22f,0.38f}); drawCube(sh,m,BOOK[p]); }
    }
    // Mini fridge
    { float fx=RX+RW*0.5f-0.35f, fz=RZ+1.0f;
      m=glm::scale(glm::translate(glm::mat4(1.f),{fx,FY+0.55f,fz}),{0.60f,1.10f,0.58f}); drawCube(sh,m,glm::vec3(0.82f,0.82f,0.84f),texConcrete,1.f);
      glm::vec3 led(0.10f,0.85f,0.20f); glUniform3fv(glGetUniformLocation(sh,"emissive"),1,glm::value_ptr(led));
      m=glm::scale(glm::translate(glm::mat4(1.f),{fx-0.31f,FY+0.30f,fz-0.20f}),{0.04f,0.05f,0.05f}); drawCube(sh,m,led);
      glUniform3fv(glGetUniformLocation(sh,"emissive"),1,glm::value_ptr(ZERO3));
    }
    // Bedside table + shared lamp
    { float bx2=RX, bz2=RZ+0.5f;
      m=glm::scale(glm::translate(glm::mat4(1.f),{bx2,FY+0.38f,bz2}),{0.70f,0.76f,0.70f}); drawCube(sh,m,TAB_C,texWood,1.f);
      { glm::mat4 bs=glm::translate(glm::mat4(1.f),{bx2,FY+1.08f,bz2}); bs=glm::rotate(bs,glm::radians(180.f),{1.f,0.f,0.f}); bs=glm::scale(bs,{0.26f,0.32f,0.26f}); drawCone(sh,bs,glm::vec3(0.85f,0.75f,0.48f)); }
      glm::vec3 bsb(0.98f,0.90f,0.65f); glUniform3fv(glGetUniformLocation(sh,"emissive"),1,glm::value_ptr(bsb*0.65f));
      m=glm::scale(glm::translate(glm::mat4(1.f),{bx2,FY+0.96f,bz2}),{0.12f,0.12f,0.12f}); drawSphere(sh,m,bsb,texMarble,1.f);
      glUniform3fv(glGetUniformLocation(sh,"emissive"),1,glm::value_ptr(ZERO3));
    }
    // Room number plate
    { glm::vec3 pg(0.90f,0.85f,0.65f); glUniform3fv(glGetUniformLocation(sh,"emissive"),1,glm::value_ptr(pg*0.60f));
      m=glm::scale(glm::translate(glm::mat4(1.f),{RX,FY+RH-0.32f,RZ+RD*0.5f+WALL_T*0.55f}),{0.60f,0.24f,0.09f}); drawCube(sh,m,pg);
      glUniform3fv(glGetUniformLocation(sh,"emissive"),1,glm::value_ptr(ZERO3));
    }
}
