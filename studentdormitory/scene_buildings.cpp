/*
 * scene_buildings.cpp  –  Complete rewrite with all fixes
 *
 * LAYOUT (corrected):
 *   LEFT  BUILDING  x=-22..-10  z=-16..+16  Student Living (2 floors, FULL WALLS)
 *   RIGHT BUILDING  x=+10..+22  z=-16..+16  Common Facilities (2 floors, FULL WALLS)
 *   COURTYARD       x=-10..+10  z=-16..+16  grass, benches, trees
 *   STAIRCASE       RIGHT side of gate (x=+4..+10, z=+8..+14) – steps going UP
 *   GATE            z=+28, centred x=0
 *   BOUNDARY WALL   aligned exactly with gate x extent ±28
 *   TENNIS COURT    x=-52, z=-10  (LEFT of campus)
 *   FOOTBALL FIELD  x=+52, z=-10  (RIGHT of campus)
 *
 * Interior: renderIntegratedRoom() is called when the camera enters the building.
 * No separate "O key" room – this is inline inside renderDormBlockA().
 */

#include "scene_buildings.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

 // ── Material palette ─────────────────────────────────────────────────────────
static const glm::vec3 BRICK(0.62f, 0.28f, 0.18f);
static const glm::vec3 CONC(0.80f, 0.80f, 0.78f);
static const glm::vec3 ROOF_C(0.68f, 0.68f, 0.66f);
static const glm::vec3 TRIM(0.55f, 0.55f, 0.53f);
static const glm::vec3 CREAM(0.94f, 0.91f, 0.84f);
static const glm::vec3 FLOOR_C(0.78f, 0.72f, 0.64f);
static const glm::vec3 WOOD_D(0.36f, 0.22f, 0.10f);
static const glm::vec3 WOOD_M(0.58f, 0.40f, 0.20f);
static const glm::vec3 WOOD_L(0.74f, 0.56f, 0.32f);
static const glm::vec3 IRON(0.22f, 0.22f, 0.24f);
static const glm::vec3 WARM_W(1.00f, 0.96f, 0.82f);
static const glm::vec3 SOFA_C(0.72f, 0.64f, 0.52f);
static const glm::vec3 PLANT(0.20f, 0.52f, 0.18f);
static const glm::vec3 ZERO3(0.f, 0.f, 0.f);

// ── Helpers ───────────────────────────────────────────────────────────────────
static void wallLight(unsigned int sh, float x, float y, float z, int faceDir) {
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,y,z }), { 0.08f,0.22f,0.22f });
    drawCube(sh, m, TRIM);
    float armX = x + faceDir * 0.18f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { armX,y,z }), { 0.30f,0.06f,0.06f });
    drawCube(sh, m, TRIM);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x + faceDir * 0.34f,y - 0.12f,z }), { 0.14f,0.22f,0.14f });
    drawCylinder(sh, m, glm::vec3(0.88f, 0.84f, 0.55f));
    if (!dayMode) {
        glm::vec3 em(0.90f, 0.82f, 0.45f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(em));
        m = glm::scale(glm::translate(glm::mat4(1.f), { x + faceDir * 0.34f,y - 0.12f,z }), { 0.10f,0.16f,0.10f });
        drawSphere(sh, m, WARM_W, texMarble, 1.f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    }
}

static void pendantLight(unsigned int sh, float x, float y, float z) {
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,y,z }), { 0.24f,0.04f,0.24f });
    drawCylinder(sh, m, TRIM);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,y - 0.22f,z }), { 0.03f,0.38f,0.03f });
    drawCylinder(sh, m, IRON);
    {
        glm::mat4 sh2 = glm::translate(glm::mat4(1.f), { x,y - 0.44f,z });
        sh2 = glm::rotate(sh2, glm::radians(180.f), { 1.f,0.f,0.f });
        sh2 = glm::scale(sh2, { 0.32f,0.28f,0.32f });
        drawCone(sh, sh2, glm::vec3(0.72f, 0.68f, 0.58f));
    }
    glm::vec3 bulb = WARM_W;
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(bulb * 0.85f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,y - 0.40f,z }), { 0.10f,0.10f,0.10f });
    drawSphere(sh, m, bulb, texMarble, 1.f);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
}

static void drainPipe(unsigned int sh, float x, float z, float h) {
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { x,h * 0.5f,z }), { 0.10f,h,0.10f });
    drawCylinder(sh, m, glm::vec3(0.45f, 0.45f, 0.46f));
}

static void decorTree(unsigned int sh, float x, float z) {
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,0.22f,z }), { 1.10f,0.44f,1.10f });
    drawCube(sh, m, BRICK, texBrick, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,1.20f,z }), { 0.20f,1.60f,0.20f });
    drawCylinder(sh, m, COL_TRUNK, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,2.85f,z }), { 1.80f,1.80f,1.80f });
    drawSphere(sh, m, glm::vec3(0.25f, 0.52f, 0.18f), texGrass, 2.f);
}

// ── Integrated dormitory room interior (rendered when camera is inside) ───────
static void renderIntegratedRoom(unsigned int sh,
    float roomX0, float roomX1,
    float roomZ0, float roomZ1,
    float FY, float CLG) {
    float xc = (roomX0 + roomX1) * 0.5f, zc = (roomZ0 + roomZ1) * 0.5f;
    float rw = roomX1 - roomX0, rd = roomZ1 - roomZ0;
    const float WT = 0.28f;

    // Floor & ceiling
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY - 0.06f,zc }), { rw,0.12f,rd });
    drawCube(sh, m, glm::vec3(0.62f, 0.46f, 0.24f), texWood, 5.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + CLG + 0.06f,zc }), { rw,0.12f,rd });
    drawCube(sh, m, CREAM, texConcrete, 3.f);

    // Four walls (south has window, north has door, east & west solid)
    // South wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + CLG * 0.5f,roomZ0 - WT * 0.5f }), { rw + WT * 2,CLG,WT });
    drawCube(sh, m, CREAM, texConcrete, 2.f);
    // Window in south wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + CLG * 0.58f,roomZ0 - WT * 0.5f }), { 2.6f,1.7f,WT + 0.04f });
    drawCrystalGlass(sh, m);
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + CLG * 0.58f + 0.92f,roomZ0 - WT * 0.5f }), { 2.8f,0.20f,WT + 0.06f });
    drawCube(sh, m, TRIM, texConcrete, 1.f);

    // North wall (door gap)
    float dw = 1.80f, dh = 2.30f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc - (rw - dw) * 0.25f,FY + CLG * 0.5f,roomZ1 + WT * 0.5f }), { (rw - dw) * 0.5f,CLG,WT });
    drawCube(sh, m, CREAM, texConcrete, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc + (rw - dw) * 0.25f,FY + CLG * 0.5f,roomZ1 + WT * 0.5f }), { (rw - dw) * 0.5f,CLG,WT });
    drawCube(sh, m, CREAM, texConcrete, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + dh + (CLG - dh) * 0.5f,roomZ1 + WT * 0.5f }), { dw,CLG - dh,WT });
    drawCube(sh, m, CREAM, texConcrete, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + dh * 0.5f,roomZ1 + WT * 0.5f }), { dw,dh,WT + 0.04f });
    drawCube(sh, m, WOOD_D, texWood, 1.f);

    // West & East walls
    m = glm::scale(glm::translate(glm::mat4(1.f), { roomX0 - WT * 0.5f,FY + CLG * 0.5f,zc }), { WT,CLG,rd });
    drawCube(sh, m, CREAM, texConcrete, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { roomX1 + WT * 0.5f,FY + CLG * 0.5f,zc }), { WT,CLG,rd });
    drawCube(sh, m, CREAM, texConcrete, 2.f);

    // Skirting boards
    glm::vec3 sk(0.40f, 0.28f, 0.10f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + 0.08f,roomZ0 }), { rw,0.16f,0.05f }); drawCube(sh, m, sk, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + 0.08f,roomZ1 }), { rw,0.16f,0.05f }); drawCube(sh, m, sk, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { roomX0,FY + 0.08f,zc }), { 0.05f,0.16f,rd }); drawCube(sh, m, sk, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { roomX1,FY + 0.08f,zc }), { 0.05f,0.16f,rd }); drawCube(sh, m, sk, texWood, 1.f);

    // ── Ceiling pendant light ──────────────────────────────────────────────
    pendantLight(sh, xc, FY + CLG - 0.02f, zc);

    // ── Ceiling FAN (animated) ────────────────────────────────────────────
    {
        float fY2 = FY + CLG - 0.10f;
        float t = (float)glfwGetTime();
        m = glm::scale(glm::translate(glm::mat4(1.f), { xc,fY2,zc }), { 0.30f,0.18f,0.30f });
        drawCylinder(sh, m, glm::vec3(0.55f, 0.55f, 0.58f));
        for (int b = 0; b < 4; b++) {
            float ang = t * 100.f + b * 90.f;
            glm::mat4 blade = glm::translate(glm::mat4(1.f), { xc,fY2 - 0.05f,zc });
            blade = glm::rotate(blade, glm::radians(ang), { 0.f,1.f,0.f });
            blade = glm::translate(blade, { 1.0f,0.f,0.f });
            blade = glm::scale(blade, { 1.50f,0.05f,0.40f });
            drawCube(sh, blade, glm::vec3(0.70f, 0.64f, 0.52f), texWood, 1.f);
        }
        m = glm::scale(glm::translate(glm::mat4(1.f), { xc,fY2 + 0.10f,zc }), { 0.05f,0.18f,0.05f });
        drawCylinder(sh, m, glm::vec3(0.50f, 0.50f, 0.52f));
    }

    // ── BED ───────────────────────────────────────────────────────────────
    float bx = roomX0 + 1.10f, bz = roomZ0 + 2.80f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { bx,FY + 0.22f,bz }), { 2.0f,0.40f,3.90f });
    drawCube(sh, m, WOOD_D, texWood, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { bx,FY + 0.50f,bz }), { 1.80f,0.22f,3.60f });
    drawCube(sh, m, glm::vec3(0.88f, 0.84f, 0.80f), texConcrete, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { bx,FY + 0.64f,bz - 0.80f }), { 1.78f,0.14f,2.00f });
    drawCube(sh, m, glm::vec3(0.22f, 0.36f, 0.62f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { bx,FY + 0.66f,bz + 1.35f }), { 1.68f,0.16f,0.82f });
    drawCube(sh, m, glm::vec3(0.96f, 0.94f, 0.90f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { bx,FY + 1.00f,roomZ0 + 0.28f }), { 2.0f,0.95f,0.15f });
    drawCube(sh, m, WOOD_D, texWood, 1.f);

    // ── DESK + CHAIR ──────────────────────────────────────────────────────
    float dx = roomX1 - 0.90f, dz = zc - 1.0f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { dx,FY + 0.76f,dz }), { 1.60f,0.07f,0.80f });
    drawCube(sh, m, WOOD_M, texWood, 1.f);
    for (int li : {-1, 1}) for (int lz2 : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { dx + li * 0.68f,FY + 0.37f,dz + lz2 * 0.32f }), { 0.09f,0.74f,0.09f });
        drawCylinder(sh, m, WOOD_M);
    }
    // Drawer block
    m = glm::scale(glm::translate(glm::mat4(1.f), { dx,FY + 0.38f,dz - 0.28f }), { 1.55f,0.62f,0.28f });
    drawCube(sh, m, WOOD_M, texWood, 1.f);
    // Laptop on desk
    m = glm::scale(glm::translate(glm::mat4(1.f), { dx,FY + 0.80f,dz - 0.05f }), { 0.78f,0.04f,0.55f });
    drawCube(sh, m, glm::vec3(0.15f, 0.15f, 0.17f));
    {
        glm::mat4 ls = glm::translate(glm::mat4(1.f), { dx,FY + 1.10f,dz - 0.28f });
        ls = glm::rotate(ls, glm::radians(-15.f), { 1.f,0.f,0.f });
        ls = glm::scale(ls, { 0.75f,0.48f,0.04f });
        drawCube(sh, ls, glm::vec3(0.12f, 0.12f, 0.15f));
        glm::vec3 sc(0.40f, 0.60f, 0.95f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(sc * 0.55f));
        ls = glm::translate(glm::mat4(1.f), { dx,FY + 1.10f,dz - 0.29f });
        ls = glm::rotate(ls, glm::radians(-15.f), { 1.f,0.f,0.f });
        ls = glm::scale(ls, { 0.68f,0.42f,0.03f });
        drawCube(sh, ls, sc);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    }

    // ── TABLE SPOTLIGHT (desk lamp) ───────────────────────────────────────
    // Emissive cone shade + bulb pointing at desk surface
    {
        m = glm::scale(glm::translate(glm::mat4(1.f), { dx + 0.55f,FY + 0.88f,dz - 0.25f }), { 0.05f,0.25f,0.05f });
        drawCylinder(sh, m, glm::vec3(0.50f, 0.50f, 0.52f));  // arm
        glm::mat4 sh2 = glm::translate(glm::mat4(1.f), { dx + 0.55f,FY + 1.06f,dz - 0.25f });
        sh2 = glm::rotate(sh2, glm::radians(180.f), { 1.f,0.f,0.f });
        sh2 = glm::scale(sh2, { 0.24f,0.20f,0.24f });
        drawCone(sh, sh2, glm::vec3(0.85f, 0.80f, 0.40f));  // shade
        glm::vec3 lampBulb(1.f, 0.95f, 0.70f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(lampBulb * 0.80f));
        m = glm::scale(glm::translate(glm::mat4(1.f), { dx + 0.55f,FY + 0.96f,dz - 0.25f }), { 0.08f,0.08f,0.08f });
        drawSphere(sh, m, lampBulb, texMarble, 1.f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    }

    // Chair
    float cz = dz + 0.72f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { dx,FY + 0.46f,cz }), { 0.65f,0.07f,0.65f });
    drawCube(sh, m, WOOD_L, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { dx,FY + 0.82f,cz - 0.32f }), { 0.65f,0.70f,0.08f });
    drawCube(sh, m, WOOD_L, texWood, 1.f);
    for (int li : {-1, 1}) for (int lz2 : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { dx + li * 0.26f,FY + 0.23f,cz + lz2 * 0.27f }), { 0.07f,0.46f,0.07f });
        drawCylinder(sh, m, WOOD_L);
    }

    // ── WARDROBE ──────────────────────────────────────────────────────────
    float wx = roomX1 - 0.90f, wz = roomZ1 - 0.48f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { wx,FY + 1.25f,wz }), { 1.50f,2.50f,0.72f });
    drawCube(sh, m, WOOD_L, texWood, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { wx,FY + 2.52f,wz }), { 1.62f,0.12f,0.80f });
    drawCube(sh, m, WOOD_M, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { wx,FY + 1.68f,wz - 0.37f }), { 0.58f,1.22f,0.04f });
    drawCrystalGlass(sh, m);
    for (int h : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { wx + h * 0.32f,FY + 1.30f,wz - 0.38f }), { 0.06f,0.24f,0.06f });
        drawCylinder(sh, m, glm::vec3(0.75f, 0.68f, 0.45f));
    }

    // ── BOOKSHELF on south wall ────────────────────────────────────────────
    float sx = roomX0 + 1.35f, sz = roomZ0 + 0.18f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { sx,FY + 1.88f,sz }), { 2.20f,1.25f,0.26f });
    drawCube(sh, m, WOOD_M, texWood, 1.f);
    for (int sb : {0, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { sx,FY + 1.28f + sb * 0.58f,sz + 0.11f }), { 2.20f,0.06f,0.28f });
        drawCube(sh, m, WOOD_M, texWood, 1.f);
    }
    glm::vec3 bkc[4] = { {0.72f,0.18f,0.18f},{0.18f,0.42f,0.72f},{0.24f,0.58f,0.22f},{0.70f,0.62f,0.16f} };
    for (int bk = 0; bk < 5; bk++) {
        float bkH = 0.26f + (bk % 3) * 0.05f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { sx - 0.82f + bk * 0.38f,FY + 1.28f + bkH * 0.5f,sz + 0.13f }), { 0.10f,bkH,0.26f });
        drawCube(sh, m, bkc[bk % 4]);
    }

    // ── SMALL FRIDGE (mini appliance) ─────────────────────────────────────
    {
        float fx = roomX0 + 0.35f, fz = roomZ1 - 0.40f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { fx,FY + 0.56f,fz }), { 0.60f,1.12f,0.58f });
        drawCube(sh, m, glm::vec3(0.82f, 0.82f, 0.84f), texConcrete, 1.f);
        glm::vec3 led(0.10f, 0.85f, 0.20f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(led));
        m = glm::scale(glm::translate(glm::mat4(1.f), { fx - 0.31f,FY + 0.28f,fz - 0.20f }), { 0.04f,0.05f,0.05f });
        drawCube(sh, m, led);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    }

    // ── NOTICE BOARD on east wall ─────────────────────────────────────────
    {
        float nx = roomX1 + WT * 0.5f, ny = FY + 1.55f, nz = zc + 0.4f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { nx,ny,nz }), { 0.09f,0.90f,1.40f });
        drawCube(sh, m, glm::vec3(0.68f, 0.50f, 0.28f));
        m = glm::scale(glm::translate(glm::mat4(1.f), { nx,ny,nz }), { 0.10f,1.00f,1.52f });
        drawCube(sh, m, WOOD_M);
        for (int p = 0; p < 3; p++) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { nx,ny - 0.22f + p * 0.26f,nz - 0.42f + p * 0.40f }), { 0.07f,0.20f,0.36f });
            drawCube(sh, m, bkc[p]);
        }
    }

    // ── ROOM NUMBER PLATE (emissive gold) above door ──────────────────────
    {
        glm::vec3 pg(0.90f, 0.85f, 0.65f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(pg * 0.60f));
        m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + CLG - 0.30f,roomZ1 + WT * 0.55f }), { 0.60f,0.24f,0.09f });
        drawCube(sh, m, pg);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  LEFT BUILDING – Student Living (full walls + integrated interior)
// ════════════════════════════════════════════════════════════════════════════
void renderDormBlockA(unsigned int sh) {
    const float X0 = -22.f, X1 = -10.f, XC = (X0 + X1) * 0.5f;
    const float Z0 = -16.f, Z1 = 16.f, ZC = (Z0 + Z1) * 0.5f;
    const float W = X1 - X0, D = Z1 - Z0;
    const float FH = 3.6f, TH = FH * 2.f;
    const float WALL_T = 0.35f;
    glm::mat4 m;

    // ── EXTERIOR SHELL ────────────────────────────────────────────────────
    // Ground-floor brick
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.25f,ZC }), { W,FH * 0.5f,D });
    drawCube(sh, m, BRICK, texBrick, 8.f);
    // Upper concrete
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH + FH * 0.5f,ZC }), { W,FH,D });
    drawCube(sh, m, CONC, texConcrete, 8.f);
    // Floor slab between storeys
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH,ZC }), { W + 0.1f,0.28f,D + 0.1f });
    drawCube(sh, m, TRIM, texConcrete, 5.f);
    // Flat roof
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,TH + 0.18f,ZC }), { W + 0.8f,0.38f,D + 0.8f });
    drawCube(sh, m, ROOF_C, texConcrete, 6.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,TH + 0.46f,ZC }), { W + 0.82f,0.35f,D + 0.82f });
    drawCube(sh, m, TRIM, texConcrete, 6.f);

    // ── GROUND FLOOR WALLS (full enclosure, courtyard side = X1) ─────────
    // North wall (z=Z1 face) – full
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.5f,Z1 + WALL_T * 0.5f }), { W,FH,WALL_T });
    drawCube(sh, m, BRICK, texBrick, 3.f);
    // South wall (z=Z0 face) – full
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.5f,Z0 - WALL_T * 0.5f }), { W,FH,WALL_T });
    drawCube(sh, m, BRICK, texBrick, 3.f);
    // West outer wall (x=X0) – full
    m = glm::scale(glm::translate(glm::mat4(1.f), { X0 - WALL_T * 0.5f,FH * 0.5f,ZC }), { WALL_T,FH,D });
    drawCube(sh, m, BRICK, texBrick, 5.f);
    // East courtyard wall (x=X1) – full with windows
    m = glm::scale(glm::translate(glm::mat4(1.f), { X1 + WALL_T * 0.5f,FH * 0.5f,ZC }), { WALL_T,FH,D });
    drawCube(sh, m, BRICK, texBrick, 5.f);
    // Main entrance door on courtyard face
    m = glm::scale(glm::translate(glm::mat4(1.f), { X1 + WALL_T * 0.5f,FH * 0.40f,ZC }), { WALL_T + 0.04f,FH * 0.78f,2.20f });
    drawCube(sh, m, WOOD_D, texWood, 1.f);

    // Courtyard-facing windows ground floor
    for (int i = 0; i < 3; i++) {
        float wz = Z0 + 4.f + i * 7.2f;
        if (fabsf(wz - ZC) < 1.5f) continue; // skip door zone
        m = glm::scale(glm::translate(glm::mat4(1.f), { X1 + 0.01f,FH * 0.56f,wz }), { WALL_T + 0.04f,1.80f,2.40f });
        drawCrystalGlass(sh, m);
        m = glm::scale(glm::translate(glm::mat4(1.f), { X1 + 0.01f,FH * 0.56f + 0.98f,wz }), { WALL_T + 0.06f,0.22f,2.60f });
        drawCube(sh, m, TRIM, texConcrete, 1.f);
        wallLight(sh, X1 + 0.04f, FH * 0.72f, wz, +1);
    }
    // Outer face small windows
    for (int i = 0; i < 3; i++) {
        float wz = Z0 + 5.f + i * 9.0f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { X0 + 0.01f,FH * 0.56f,wz }), { WALL_T + 0.04f,1.60f,2.20f });
        drawGlass(sh, m);
    }
    drainPipe(sh, X0 - 0.04f, Z0 + 2.f, TH + 0.5f);
    drainPipe(sh, X0 - 0.04f, Z0 + 16.f, TH + 0.5f);

    // Upper floor windows (courtyard face)
    for (int i = 0; i < 4; i++) {
        float wz = Z0 + 4.f + i * 6.8f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { X1 + 0.01f,FH + FH * 0.56f,wz }), { WALL_T + 0.04f,1.80f,2.60f });
        drawCrystalGlass(sh, m);
    }

    // ── INTERIOR ROOM (always rendered, camera sees it when inside) ────────
    {
        const float FY = 0.62f, CLG = FH - 0.52f;
        // Room occupies inner space of left half of building
        float R1_X0 = X0 + WALL_T + 0.1f, R1_X1 = XC - 0.5f;
        float R1_Z0 = Z0 + WALL_T + 0.1f, R1_Z1 = ZC;
        renderIntegratedRoom(sh, R1_X0, R1_X1, R1_Z0, R1_Z1, FY, CLG);

        // Corridor ceiling lights (right half)
        float CX1 = X1 - 2.0f, CXC = (CX1 + X1) * 0.5f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { CXC,FY + CLG + 0.06f,ZC }), { X1 - CX1,0.10f,D });
        drawCube(sh, m, CREAM, texConcrete, 4.f);
        for (int i = 0; i < 5; i++) pendantLight(sh, CXC, FY + CLG - 0.02f, Z0 + 3.5f + i * 6.0f);
    }
    {   // Upper floor corridor
        const float FY = FH + 0.62f, CLG = FH - 0.52f;
        float CX1 = X1 - 2.0f, CXC = (CX1 + X1) * 0.5f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { CXC,FY + CLG + 0.06f,ZC }), { X1 - CX1,0.10f,D });
        drawCube(sh, m, CREAM, texConcrete, 4.f);
        for (int i = 0; i < 5; i++) pendantLight(sh, CXC, FY + CLG - 0.02f, Z0 + 3.5f + i * 6.0f);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  RIGHT BUILDING – Common Facilities (full walls + interior)
// ════════════════════════════════════════════════════════════════════════════
void renderDormBlockB(unsigned int sh) {
    const float X0 = 10.f, X1 = 22.f, XC = (X0 + X1) * 0.5f;
    const float Z0 = -16.f, Z1 = 16.f, ZC = (Z0 + Z1) * 0.5f;
    const float W = X1 - X0, D = Z1 - Z0;
    const float FH = 3.6f, TH = FH * 2.f;
    const float WALL_T = 0.35f;
    glm::mat4 m;

    // Exterior
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.25f,ZC }), { W,FH * 0.5f,D }); drawCube(sh, m, BRICK, texBrick, 8.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH + FH * 0.5f,ZC }), { W,FH,D }); drawCube(sh, m, CONC, texConcrete, 8.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH,ZC }), { W + 0.1f,0.28f,D + 0.1f }); drawCube(sh, m, TRIM, texConcrete, 5.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,TH + 0.18f,ZC }), { W + 0.8f,0.38f,D + 0.8f }); drawCube(sh, m, ROOF_C, texConcrete, 6.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,TH + 0.46f,ZC }), { W + 0.82f,0.35f,D + 0.82f }); drawCube(sh, m, TRIM, texConcrete, 6.f);

    // Ground floor full walls
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.5f,Z1 + WALL_T * 0.5f }), { W,FH,WALL_T }); drawCube(sh, m, BRICK, texBrick, 3.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.5f,Z0 - WALL_T * 0.5f }), { W,FH,WALL_T }); drawCube(sh, m, BRICK, texBrick, 3.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { X1 + WALL_T * 0.5f,FH * 0.5f,ZC }), { WALL_T,FH,D }); drawCube(sh, m, BRICK, texBrick, 5.f);
    // West courtyard wall with door
    m = glm::scale(glm::translate(glm::mat4(1.f), { X0 - WALL_T * 0.5f,FH * 0.5f,ZC }), { WALL_T,FH,D }); drawCube(sh, m, BRICK, texBrick, 5.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { X0 - WALL_T * 0.5f,FH * 0.40f,ZC }), { WALL_T + 0.04f,FH * 0.78f,2.20f }); drawCube(sh, m, WOOD_D, texWood, 1.f);

    // Courtyard-facing windows
    for (int fl = 0; fl < 2; fl++) {
        float fy = fl * FH;
        for (int i = 0; i < 3; i++) {
            float wz = Z0 + 4.f + i * 7.2f;
            if (fabsf(wz - ZC) < 1.5f) continue;
            m = glm::scale(glm::translate(glm::mat4(1.f), { X0 + 0.01f,fy + FH * 0.56f,wz }), { WALL_T + 0.04f,1.80f,2.60f }); drawCrystalGlass(sh, m);
            if (fl == 0) wallLight(sh, X0 - 0.04f, FH * 0.72f, wz, -1);
        }
    }
    for (int fl = 0; fl < 2; fl++) {
        float fy = fl * FH;
        for (int i = 0; i < 3; i++) {
            float wz = Z0 + 5.f + i * 9.0f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { X1 - 0.01f,fy + FH * 0.56f,wz }), { WALL_T + 0.04f,1.60f,2.20f }); drawGlass(sh, m);
        }
    }
    drainPipe(sh, X1 + 0.04f, Z0 + 2.f, TH + 0.5f);
    drainPipe(sh, X1 + 0.04f, Z0 + 16.f, TH + 0.5f);

    // ── GROUND FLOOR INTERIOR ─────────────────────────────────────────────
    {
        const float FY = 0.62f, CLG = FH - 0.52f;
        float CRX0 = X0 + WALL_T, CRX1 = X0 + 6.0f, CRXC = (CRX0 + CRX1) * 0.5f;
        // Common room floor + ceiling
        m = glm::scale(glm::translate(glm::mat4(1.f), { CRXC,FY - 0.06f,ZC }), { CRX1 - CRX0,0.10f,D }); drawCube(sh, m, FLOOR_C, texTile, 8.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { CRXC,FY + CLG + 0.06f,ZC }), { CRX1 - CRX0,0.10f,D }); drawCube(sh, m, CREAM, texConcrete, 4.f);
        pendantLight(sh, CRXC, FY + CLG - 0.02f, ZC - 4.f);
        pendantLight(sh, CRXC, FY + CLG - 0.02f, ZC + 4.f);
        // Sofa
        float sx = CRX0 + 0.55f, sz = ZC - 2.0f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { sx,FY + 0.46f,sz }), { 0.80f,0.46f,2.60f }); drawCube(sh, m, SOFA_C);
        m = glm::scale(glm::translate(glm::mat4(1.f), { sx - 0.38f,FY + 0.72f,sz }), { 0.10f,0.50f,2.60f }); drawCube(sh, m, SOFA_C);
        sz = ZC + 2.4f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { CRXC + 0.40f,FY + 0.46f,sz }), { 2.0f,0.46f,0.80f }); drawCube(sh, m, SOFA_C);
        m = glm::scale(glm::translate(glm::mat4(1.f), { CRXC + 0.40f,FY + 0.72f,sz + 0.38f }), { 2.0f,0.50f,0.10f }); drawCube(sh, m, SOFA_C);
        // Coffee table
        m = glm::scale(glm::translate(glm::mat4(1.f), { CRXC + 0.20f,FY + 0.34f,ZC }), { 1.20f,0.08f,1.20f }); drawCube(sh, m, WOOD_M, texWood, 1.f);
        // TV (animated screen)
        {
            float tvX = CRX1 - 0.14f, tvY = FY + 1.60f, tvZ = ZC - 5.0f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { tvX,tvY,tvZ }), { 0.22f,1.68f,2.78f }); drawCube(sh, m, glm::vec3(0.08f, 0.08f, 0.09f));
            float t2 = (float)glfwGetTime(), bv = 0.25f + 0.10f * sinf(t2 * 0.5f);
            glm::vec3 scrC = { bv * 0.20f,bv * 0.45f,bv * 0.95f };
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(scrC * 0.70f));
            m = glm::scale(glm::translate(glm::mat4(1.f), { tvX - 0.02f,tvY,tvZ }), { 0.06f,1.50f,2.60f }); drawCube(sh, m, scrC);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
        }
        // Indoor plant
        {
            float px = CRX0 + 0.40f, pz2 = Z1 - WALL_T - 0.50f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { px,FY + 0.22f,pz2 }), { 0.50f,0.44f,0.50f }); drawCylinder(sh, m, BRICK, texBrick, 1.f);
            m = glm::scale(glm::translate(glm::mat4(1.f), { px,FY + 0.55f,pz2 }), { 0.08f,0.55f,0.08f }); drawCylinder(sh, m, COL_TRUNK);
            m = glm::scale(glm::translate(glm::mat4(1.f), { px,FY + 1.10f,pz2 }), { 0.80f,0.80f,0.80f }); drawSphere(sh, m, PLANT, texGrass, 1.f);
        }

        // Reading room
        float RRX0 = CRX1 + WALL_T, RRX1 = X1 - WALL_T, RRXC = (RRX0 + RRX1) * 0.5f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { RRXC,FY - 0.06f,ZC }), { RRX1 - RRX0,0.10f,D }); drawCube(sh, m, glm::vec3(0.74f, 0.68f, 0.58f), texWood, 6.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { RRXC,FY + CLG + 0.06f,ZC }), { RRX1 - RRX0,0.10f,D }); drawCube(sh, m, CREAM, texConcrete, 4.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { CRX1,FY + CLG * 0.5f,ZC }), { WALL_T,CLG,D }); drawCube(sh, m, CREAM, texConcrete, 3.f);
        pendantLight(sh, RRXC, FY + CLG - 0.02f, ZC - 5.f);
        pendantLight(sh, RRXC, FY + CLG - 0.02f, ZC + 5.f);
        // Bookshelves
        for (int s = 0; s < 2; s++) {
            float bsx = X1 - 0.20f, bsz = Z0 + 4.0f + s * 8.0f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { bsx,FY + 1.80f,bsz }), { 0.26f,1.50f,3.20f }); drawCube(sh, m, WOOD_M, texWood, 1.f);
        }
        // Reading desks
        for (int row = 0; row < 2; row++) {
            float dz2 = ZC - 7.5f + row * 6.5f;
            for (int col = 0; col < 2; col++) {
                float dx2 = RRXC - 1.4f + col * 2.8f;
                m = glm::scale(glm::translate(glm::mat4(1.f), { dx2,FY + 0.75f,dz2 }), { 1.70f,0.07f,0.80f }); drawCube(sh, m, WOOD_L, texWood, 1.f);
                for (int li : {-1, 1}) { m = glm::scale(glm::translate(glm::mat4(1.f), { dx2 + li * 0.72f,FY + 0.36f,dz2 }), { 0.08f,0.72f,0.08f }); drawCylinder(sh, m, WOOD_D); }
                // Desk lamp
                m = glm::scale(glm::translate(glm::mat4(1.f), { dx2 + 0.58f,FY + 0.88f,dz2 - 0.26f }), { 0.05f,0.24f,0.05f }); drawCylinder(sh, m, glm::vec3(0.50f, 0.50f, 0.52f));
                m = glm::scale(glm::translate(glm::mat4(1.f), { dx2,FY + 0.46f,dz2 + 0.68f }), { 0.62f,0.07f,0.62f }); drawCube(sh, m, WOOD_L, texWood, 1.f);
            }
        }
    }

    // Upper floor dining hall
    {
        const float FY = FH + 0.62f, CLG = FH - 0.52f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY - 0.06f,ZC }), { W,0.10f,D }); drawCube(sh, m, glm::vec3(0.78f, 0.73f, 0.62f), texTile, 8.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY + CLG + 0.06f,ZC }), { W,0.10f,D }); drawCube(sh, m, CREAM, texConcrete, 5.f);
        for (int i = 0; i < 4; i++) {
            float pz = Z0 + 4.0f + i * 7.5f;
            pendantLight(sh, XC - 1.5f, FY + CLG - 0.02f, pz);
            pendantLight(sh, XC + 1.5f, FY + CLG - 0.02f, pz);
        }
        for (int row = 0; row < 3; row++) {
            float tz = Z0 + 4.5f + row * 9.0f;
            for (int col = 0; col < 2; col++) {
                float tx = XC - 2.8f + col * 5.6f;
                m = glm::scale(glm::translate(glm::mat4(1.f), { tx,FY + 0.76f,tz }), { 2.40f,0.08f,1.20f }); drawCube(sh, m, WOOD_L, texWood, 1.f);
                for (int li : {-1, 1}) for (int lz2 : {-1, 1}) { m = glm::scale(glm::translate(glm::mat4(1.f), { tx + li * 1.02f,FY + 0.37f,tz + lz2 * 0.46f }), { 0.09f,0.74f,0.09f }); drawCylinder(sh, m, WOOD_D); }
                for (int side : {-1, 1}) for (int ci = 0; ci < 2; ci++) {
                    float cz2 = tz + (ci == 0 ? -0.90f : 0.90f);
                    m = glm::scale(glm::translate(glm::mat4(1.f), { tx,FY + 0.45f,cz2 }), { 0.60f,0.07f,0.60f }); drawCube(sh, m, WOOD_L, texWood, 1.f);
                }
            }
        }
        float kz = Z1 - 0.60f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY + 0.90f,kz }), { W - 1.0f,0.10f,0.80f }); drawCube(sh, m, CONC, texConcrete, 3.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY + 0.50f,kz }), { W - 1.0f,0.80f,0.78f }); drawCube(sh, m, CREAM, texConcrete, 3.f);
    }
}

// ── Corridor (covered walkways) ───────────────────────────────────────────────
void renderCorridor(unsigned int sh) {
    const float CANY = 3.60f, CAND = 1.60f, Z0 = -16.f, Z1 = 16.f;
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { -10.f + CAND * 0.5f,CANY + 0.14f,0.f }), { CAND,0.28f,Z1 - Z0 }); drawCube(sh, m, ROOF_C, texConcrete, 5.f);
    for (int i = 0; i < 6; i++) {
        float cz = Z0 + 2.5f + i * 5.5f;
        glm::mat4 cy = glm::scale(glm::translate(glm::mat4(1.f), { -10.f + CAND,CANY * 0.5f,cz }), { 0.28f,CANY,0.28f });
        drawCylinder(sh, cy, CONC, texConcrete, 1.f);
    }
    m = glm::scale(glm::translate(glm::mat4(1.f), { 10.f - CAND * 0.5f,CANY + 0.14f,0.f }), { CAND,0.28f,Z1 - Z0 }); drawCube(sh, m, ROOF_C, texConcrete, 5.f);
    for (int i = 0; i < 6; i++) {
        float cz = Z0 + 2.5f + i * 5.5f;
        glm::mat4 cy = glm::scale(glm::translate(glm::mat4(1.f), { 10.f - CAND,CANY * 0.5f,cz }), { 0.28f,CANY,0.28f });
        drawCylinder(sh, cy, CONC, texConcrete, 1.f);
    }
}

// ── Courtyard ─────────────────────────────────────────────────────────────────
void renderCourtyard(unsigned int sh) {
    glm::mat4 m;
    // Grass
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f,0.01f,0.f }), { 19.0f,0.08f,31.0f }); drawCube(sh, m, COL_GRASS, texGrass, 6.f);
    // Central path
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f,0.04f,7.0f }), { 4.0f,0.06f,42.0f }); drawCube(sh, m, COL_PLAZA, texTile, 8.f);
    // Decorative trees
    for (int s : {-1, 1}) for (int i = 0; i < 4; i++) decorTree(sh, s * 4.5f, -10.f + i * 6.5f);

    // Benches
    auto bench = [&](float x, float z, float rotY) {
        glm::mat4 base = glm::rotate(glm::translate(glm::mat4(1.f), { x,0.72f,z }), glm::radians(rotY), { 0,1,0 });
        drawCube(sh, glm::scale(glm::translate(base, { 0.f,0.f,0.f }), { 2.0f,0.11f,0.50f }), WOOD_L, texWood, 1.f);
        for (int s : {-1, 1}) { drawCube(sh, glm::scale(glm::translate(base, { s * 0.80f,-0.30f,0.18f }), { 0.10f,0.60f,0.10f }), TRIM); drawCube(sh, glm::scale(glm::translate(base, { s * 0.80f,-0.30f,-0.18f }), { 0.10f,0.60f,0.10f }), TRIM); }
        drawCube(sh, glm::scale(glm::translate(base, { 0.f,0.38f,-0.22f }), { 2.0f,0.44f,0.08f }), WOOD_L, texWood, 1.f);
        };
    for (int i = 0; i < 3; i++) { bench(-3.0f, -8.0f + i * 6.5f, 15.f); bench(3.0f, -8.0f + i * 6.5f, -15.f); }

    // ── STAIRCASE on RIGHT side (x=+4..+10, z=+8..+14), steps going south→north UP ──
    const float SX = 7.f, SZ0 = 8.0f;
    const float SWIDTH = 6.0f, RISE = 0.22f, RUN = 0.70f;
    for (int step = 0; step < 8; step++) {
        float sy = (step + 1) * RISE, sz = SZ0 + step * RUN;
        m = glm::scale(glm::translate(glm::mat4(1.f), { SX,sy * 0.5f,sz }), { SWIDTH,sy,RUN });
        drawCube(sh, m, CONC, texConcrete, 2.f);
    }
    // Back retaining wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { SX,1.0f,SZ0 - 0.35f }), { SWIDTH,2.0f,0.35f }); drawCube(sh, m, BRICK, texBrick, 2.f);
    // Railings
    for (int s : {-1, 1}) {
        float rx = SX + s * (SWIDTH * 0.5f - 0.10f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { rx,0.50f,SZ0 }), { 0.08f,1.0f,0.08f }); drawCylinder(sh, m, IRON);
        float topZ = SZ0 + 7 * RUN, topY = 8 * RISE + 0.50f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { rx,topY * 0.5f,topZ }), { 0.08f,topY,0.08f }); drawCylinder(sh, m, IRON);
        glm::mat4 rail = glm::translate(glm::mat4(1.f), { rx,(topY + 0.50f) * 0.5f,(SZ0 + topZ) * 0.5f });
        float ang = atanf((8 * RISE) / (7 * RUN));
        rail = glm::rotate(rail, ang, { 1.f,0.f,0.f });
        float rLen = sqrtf((7 * RUN) * (7 * RUN) + (8 * RISE - 0.50f) * (8 * RISE - 0.50f));
        rail = glm::scale(rail, { 0.08f,0.08f,rLen });
        drawCube(sh, rail, IRON);
    }
    m = glm::scale(glm::translate(glm::mat4(1.f), { SX,8 * RISE + 0.08f,SZ0 + 7 * RUN + 1.0f }), { SWIDTH,0.16f,2.5f }); drawCube(sh, m, CONC, texConcrete, 2.f);
}

// ── Admin annex ───────────────────────────────────────────────────────────────
void renderAdminAnnex(unsigned int sh) {
    const float AX = 20.f, AZ = -20.f;
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX,1.80f,AZ }), { 5.0f,3.60f,5.0f }); drawCube(sh, m, BRICK, texBrick, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX,3.72f,AZ }), { 5.2f,0.28f,5.2f }); drawCube(sh, m, ROOF_C, texConcrete, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX,1.20f,AZ + 2.52f }), { 1.50f,2.40f,0.22f }); drawCube(sh, m, WOOD_D, texWood, 1.f);
}

// ── Entrance sphere (gate pillar finials) ─────────────────────────────────────
void renderEntranceSphere(unsigned int sh) {
    for (int s : {-1, 1}) {
        float px = s * (4.80f + 0.90f);
        glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { px,5.80f,28.f }), { 0.55f,0.55f,0.55f });
        drawSphere(sh, m, glm::vec3(0.90f, 0.82f, 0.62f), texMarble, 1.f);
        if (!dayMode) {
            glm::vec3 em(0.90f, 0.80f, 0.40f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(em * 0.70f));
            drawSphere(sh, m, em, texMarble, 1.f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
        }
    }
}

// ── RoomInterior stub (kept for API compat) ────────────────────────────────────
void renderRoomInterior(unsigned int sh) {}

// ── renderDormRoom stub (kept for API compat – O key removed) ─────────────────
void renderDormRoom(unsigned int sh) {}