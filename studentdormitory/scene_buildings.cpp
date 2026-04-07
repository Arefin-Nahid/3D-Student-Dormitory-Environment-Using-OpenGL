/*
 * scene_buildings.cpp  –  Full Interior Dormitory System
 *
 * LEFT BUILDING  x=-22..-10  z=-16..+16  Student Living (2 floors)
 *   Ground floor: 4 dorm rooms + central corridor + 2 internal staircases
 *   Upper  floor: 4 dorm rooms + central corridor + staircase landings
 *   Each room: bed, study desk+chair, wardrobe, bookshelf, mini-fridge
 *
 * RIGHT BUILDING  x=+10..+22  z=-16..+16  Common Facilities (2 floors)
 *   Ground floor: Reading Room (west half) + Common Room (east half)
 *   Upper  floor: Dining Hall (full width) + Serving Counter
 *
 * CONSTRAINTS PRESERVED:
 *   - Exterior shell (walls, windows, roof) unchanged
 *   - Boundary walls unchanged
 *   - External central staircase unchanged
 */

#include "scene_buildings.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

 // ─── Material palette ─────────────────────────────────────────────────────────
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
static const glm::vec3 WHITE(0.96f, 0.96f, 0.94f);
static const glm::vec3 STEEL(0.72f, 0.72f, 0.74f);
static const glm::vec3 COUNTER(0.84f, 0.80f, 0.74f);
static const glm::vec3 BLUE_SHEET(0.22f, 0.36f, 0.62f);
static const glm::vec3 BOOK_RED(0.72f, 0.18f, 0.18f);
static const glm::vec3 BOOK_BLU(0.18f, 0.42f, 0.72f);
static const glm::vec3 BOOK_GRN(0.24f, 0.58f, 0.22f);
static const glm::vec3 BOOK_YEL(0.70f, 0.62f, 0.16f);

// ─── Utility helpers ──────────────────────────────────────────────────────────

static void pendantLight(unsigned int sh, float x, float y, float z) {
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,y,z }), { 0.24f,0.04f,0.24f });
    drawCylinder(sh, m, TRIM);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,y - 0.22f,z }), { 0.03f,0.38f,0.03f });
    drawCylinder(sh, m, IRON);
    {
        glm::mat4 s2 = glm::translate(glm::mat4(1.f), { x,y - 0.44f,z });
        s2 = glm::rotate(s2, glm::radians(180.f), { 1.f,0.f,0.f });
        s2 = glm::scale(s2, { 0.32f,0.28f,0.32f });
        drawCone(sh, s2, glm::vec3(0.72f, 0.68f, 0.58f));
    }
    glm::vec3 bulb = WARM_W;
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(bulb * 0.85f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,y - 0.40f,z }), { 0.10f,0.10f,0.10f });
    drawSphere(sh, m, bulb, texMarble, 1.f);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
}

static void wallLight(unsigned int sh, float x, float y, float z, int faceDir) {
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,y,z }), { 0.08f,0.22f,0.22f });
    drawCube(sh, m, TRIM);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x + faceDir * 0.18f,y,z }), { 0.30f,0.06f,0.06f });
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
    drawSphere(sh, m, PLANT, texGrass, 2.f);
}

// ─── Desk lamp helper ─────────────────────────────────────────────────────────
static void deskLamp(unsigned int sh, float x, float y, float z) {
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,y,z }), { 0.05f,0.25f,0.05f });
    drawCylinder(sh, m, STEEL);
    glm::mat4 s2 = glm::translate(glm::mat4(1.f), { x,y + 0.18f,z });
    s2 = glm::rotate(s2, glm::radians(180.f), { 1.f,0.f,0.f });
    s2 = glm::scale(s2, { 0.24f,0.20f,0.24f });
    drawCone(sh, s2, glm::vec3(0.85f, 0.80f, 0.40f));
    glm::vec3 lb(1.f, 0.95f, 0.70f);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(lb * 0.80f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,y + 0.08f,z }), { 0.07f,0.07f,0.07f });
    drawSphere(sh, m, lb, texMarble, 1.f);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
}

// ─── Chair helper ─────────────────────────────────────────────────────────────
static void studyChair(unsigned int sh, float x, float y, float z, float rotY = 0.f) {
    glm::mat4 base = glm::rotate(glm::translate(glm::mat4(1.f), { x,y,z }), glm::radians(rotY), { 0,1,0 });
    // seat
    glm::mat4 m = glm::scale(glm::translate(base, { 0.f,0.46f,0.f }), { 0.65f,0.07f,0.65f });
    drawCube(sh, m, WOOD_L, texWood, 1.f);
    // back
    m = glm::scale(glm::translate(base, { 0.f,0.82f,-0.30f }), { 0.65f,0.70f,0.07f });
    drawCube(sh, m, WOOD_L, texWood, 1.f);
    // legs
    for (int lx : {-1, 1}) for (int lz : {-1, 1}) {
        m = glm::scale(glm::translate(base, { lx * 0.26f,0.22f,lz * 0.27f }), { 0.07f,0.46f,0.07f });
        drawCylinder(sh, m, WOOD_L);
    }
}

// ─── Bookshelf helper ─────────────────────────────────────────────────────────
static void bookShelf(unsigned int sh, float x, float y, float z, float scaleX = 1.f, bool facingZ = true) {
    glm::mat4 m;
    // frame
    if (facingZ) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { x,y + 1.20f,z }), { scaleX * 2.20f,2.40f,0.28f });
    }
    else {
        m = glm::scale(glm::translate(glm::mat4(1.f), { x,y + 1.20f,z }), { 0.28f,2.40f,scaleX * 2.20f });
    }
    drawCube(sh, m, WOOD_M, texWood, 1.f);
    // shelves
    for (int s = 0; s < 3; s++) {
        float sy = y + 0.42f + s * 0.72f;
        if (facingZ) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { x,sy,z + 0.12f }), { scaleX * 2.20f,0.06f,0.30f });
        }
        else {
            m = glm::scale(glm::translate(glm::mat4(1.f), { x + 0.12f,sy,z }), { 0.30f,0.06f,scaleX * 2.20f });
        }
        drawCube(sh, m, WOOD_M, texWood, 1.f);
    }
    // books
    glm::vec3 bkc[4] = { BOOK_RED,BOOK_BLU,BOOK_GRN,BOOK_YEL };
    int nb = (int)(scaleX * 5.f + 0.5f);
    for (int bk = 0; bk < nb; bk++) {
        float bkH = 0.26f + (bk % 3) * 0.05f;
        float off = -scaleX * 1.0f + bk * (scaleX * 2.0f / (nb - 1));
        if (facingZ) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { x + off,y + 0.55f + bkH * 0.5f,z + 0.12f }), { 0.10f,bkH,0.26f });
        }
        else {
            m = glm::scale(glm::translate(glm::mat4(1.f), { x + 0.12f,y + 0.55f + bkH * 0.5f,z + off }), { 0.26f,bkH,0.10f });
        }
        drawCube(sh, m, bkc[bk % 4]);
    }
}

// ─── Full dorm room (bed + desk + wardrobe + bookshelf + fridge) ──────────────
static void dormRoom(unsigned int sh,
    float roomX0, float roomX1,
    float roomZ0, float roomZ1,
    float FY, float CLG, int roomNum, bool mirrorX = false)
{
    float xc = (roomX0 + roomX1) * 0.5f, zc = (roomZ0 + roomZ1) * 0.5f;
    float rw = roomX1 - roomX0, rd = roomZ1 - roomZ0;
    const float WT = 0.22f;
    glm::mat4 m;

    // ── Floor (wood) ────────────────────────────────────────────────────
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY - 0.06f,zc }), { rw,0.12f,rd });
    drawCube(sh, m, glm::vec3(0.62f, 0.46f, 0.24f), texWood, 5.f);

    // ── Ceiling ─────────────────────────────────────────────────────────
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + CLG + 0.06f,zc }), { rw,0.12f,rd });
    drawCube(sh, m, CREAM, texConcrete, 3.f);

    // ── Ceiling pendant light ────────────────────────────────────────────
    pendantLight(sh, xc, FY + CLG - 0.02f, zc);

    // ── Ceiling fan ─────────────────────────────────────────────────────
    {
        float fY2 = FY + CLG - 0.10f;
        float t = (float)glfwGetTime();
        m = glm::scale(glm::translate(glm::mat4(1.f), { xc,fY2,zc }), { 0.28f,0.18f,0.28f });
        drawCylinder(sh, m, STEEL);
        for (int b = 0; b < 4; b++) {
            float ang = t * 100.f + b * 90.f;
            glm::mat4 blade = glm::translate(glm::mat4(1.f), { xc,fY2 - 0.05f,zc });
            blade = glm::rotate(blade, glm::radians(ang), { 0.f,1.f,0.f });
            blade = glm::translate(blade, { 1.0f,0.f,0.f });
            blade = glm::scale(blade, { 1.50f,0.05f,0.38f });
            drawCube(sh, blade, WOOD_L, texWood, 1.f);
        }
        m = glm::scale(glm::translate(glm::mat4(1.f), { xc,fY2 + 0.10f,zc }), { 0.05f,0.18f,0.05f });
        drawCylinder(sh, m, STEEL);
    }

    // ── Interior walls (partition only, exterior walls handled in block) ──
    // South partition
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + CLG * 0.5f,roomZ0 - WT * 0.5f }), { rw + WT * 2,CLG,WT });
    drawCube(sh, m, CREAM, texConcrete, 2.f);
    // North partition (with door gap)
    {
        float dw = 1.60f, dh = 2.20f;
        float wallLeft = (rw - dw) * 0.5f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { xc - wallLeft * 0.5f - dw * 0.25f,FY + CLG * 0.5f,roomZ1 + WT * 0.5f }), { wallLeft,CLG,WT });
        drawCube(sh, m, CREAM, texConcrete, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { xc + wallLeft * 0.5f + dw * 0.25f,FY + CLG * 0.5f,roomZ1 + WT * 0.5f }), { wallLeft,CLG,WT });
        drawCube(sh, m, CREAM, texConcrete, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + dh + (CLG - dh) * 0.5f,roomZ1 + WT * 0.5f }), { dw,CLG - dh,WT });
        drawCube(sh, m, CREAM, texConcrete, 1.f);
        // Door leaf (open 30° toward corridor)
        {
            glm::mat4 dbase = glm::translate(glm::mat4(1.f), { xc - dw * 0.5f,FY + dh * 0.5f,roomZ1 + WT * 0.5f });
            dbase = glm::rotate(dbase, glm::radians(-30.f), { 0,1,0 });
            m = glm::scale(dbase, { dw,dh,WT + 0.04f });
            drawCube(sh, m, WOOD_D, texWood, 1.f);
        }
    }
    // West + East partitions (shared between rooms, only draw thin separator)
    m = glm::scale(glm::translate(glm::mat4(1.f), { roomX0 - WT * 0.5f,FY + CLG * 0.5f,zc }), { WT,CLG,rd });
    drawCube(sh, m, CREAM, texConcrete, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { roomX1 + WT * 0.5f,FY + CLG * 0.5f,zc }), { WT,CLG,rd });
    drawCube(sh, m, CREAM, texConcrete, 2.f);

    // Skirting
    glm::vec3 sk(0.40f, 0.28f, 0.10f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + 0.08f,roomZ0 }), { rw,0.14f,0.05f }); drawCube(sh, m, sk, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + 0.08f,roomZ1 }), { rw,0.14f,0.05f }); drawCube(sh, m, sk, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { roomX0,FY + 0.08f,zc }), { 0.05f,0.14f,rd });  drawCube(sh, m, sk, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { roomX1,FY + 0.08f,zc }), { 0.05f,0.14f,rd });  drawCube(sh, m, sk, texWood, 1.f);

    // Window (south wall, always outer face)
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + CLG * 0.55f,roomZ0 - WT * 0.5f }), { rw * 0.55f,1.60f,WT + 0.04f });
    drawCrystalGlass(sh, m);
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + CLG * 0.55f + 0.88f,roomZ0 - WT * 0.5f }), { rw * 0.60f,0.18f,WT + 0.06f });
    drawCube(sh, m, TRIM, texConcrete, 1.f);

    // Room number plate
    {
        glm::vec3 pg(0.90f, 0.85f, 0.65f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(pg * 0.55f));
        m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + CLG - 0.28f,roomZ1 + WT * 0.55f }), { 0.55f,0.22f,0.09f });
        drawCube(sh, m, pg);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    }

    // ── BED (against south or west wall) ───────────────────────────────────
    float bx = mirrorX ? (roomX1 - 1.10f) : (roomX0 + 1.10f);
    float bz = roomZ0 + 2.60f;
    // Frame
    m = glm::scale(glm::translate(glm::mat4(1.f), { bx,FY + 0.22f,bz }), { 2.0f,0.38f,3.80f });
    drawCube(sh, m, WOOD_D, texWood, 2.f);
    // Mattress
    m = glm::scale(glm::translate(glm::mat4(1.f), { bx,FY + 0.48f,bz }), { 1.80f,0.18f,3.50f });
    drawCube(sh, m, WHITE, texConcrete, 1.f);
    // Blanket
    m = glm::scale(glm::translate(glm::mat4(1.f), { bx,FY + 0.60f,bz - 0.60f }), { 1.78f,0.12f,2.20f });
    drawCube(sh, m, BLUE_SHEET);
    // Pillow
    m = glm::scale(glm::translate(glm::mat4(1.f), { bx,FY + 0.62f,roomZ0 + 0.90f }), { 1.65f,0.14f,0.70f });
    drawCube(sh, m, WHITE);
    // Headboard
    m = glm::scale(glm::translate(glm::mat4(1.f), { bx,FY + 0.90f,roomZ0 + 0.24f }), { 2.0f,0.85f,0.14f });
    drawCube(sh, m, WOOD_D, texWood, 1.f);
    // Bedside table
    float btx = mirrorX ? (bx + 1.20f) : (bx - 1.20f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { btx,FY + 0.36f,roomZ0 + 0.80f }), { 0.52f,0.72f,0.52f });
    drawCube(sh, m, WOOD_M, texWood, 1.f);
    // Bedside lamp
    m = glm::scale(glm::translate(glm::mat4(1.f), { btx,FY + 0.78f,roomZ0 + 0.80f }), { 0.12f,0.28f,0.12f });
    drawCylinder(sh, m, CREAM);
    glm::vec3 blamp(1.f, 0.90f, 0.65f);
    if (!dayMode) glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(blamp * 0.5f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { btx,FY + 0.96f,roomZ0 + 0.80f }), { 0.20f,0.16f,0.20f });
    drawCone(sh, m, glm::vec3(0.88f, 0.84f, 0.62f));
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));

    // ── STUDY DESK + CHAIR ─────────────────────────────────────────────────
    float dx = mirrorX ? (roomX0 + 0.88f) : (roomX1 - 0.88f);
    float dz = zc + 0.40f;
    // Desk surface
    m = glm::scale(glm::translate(glm::mat4(1.f), { dx,FY + 0.76f,dz }), { 1.55f,0.07f,0.78f });
    drawCube(sh, m, WOOD_M, texWood, 1.f);
    // Desk legs
    for (int lx : {-1, 1}) for (int lz2 : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { dx + lx * 0.64f,FY + 0.37f,dz + lz2 * 0.30f }), { 0.08f,0.72f,0.08f });
        drawCylinder(sh, m, WOOD_M);
    }
    // Drawer unit
    m = glm::scale(glm::translate(glm::mat4(1.f), { dx,FY + 0.36f,dz - 0.25f }), { 1.50f,0.60f,0.28f });
    drawCube(sh, m, WOOD_M, texWood, 1.f);
    // Drawer handles
    for (int i : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { dx + i * 0.50f,FY + 0.36f,dz - 0.40f }), { 0.22f,0.05f,0.05f });
        drawCube(sh, m, STEEL);
    }
    // Laptop
    m = glm::scale(glm::translate(glm::mat4(1.f), { dx,FY + 0.80f,dz - 0.02f }), { 0.72f,0.04f,0.50f });
    drawCube(sh, m, glm::vec3(0.15f, 0.15f, 0.17f));
    {
        glm::mat4 ls = glm::translate(glm::mat4(1.f), { dx,FY + 1.08f,dz - 0.26f });
        ls = glm::rotate(ls, glm::radians(-15.f), { 1.f,0.f,0.f });
        ls = glm::scale(ls, { 0.70f,0.44f,0.04f });
        drawCube(sh, ls, glm::vec3(0.12f, 0.12f, 0.15f));
        glm::vec3 sc(0.40f, 0.60f, 0.95f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(sc * 0.50f));
        ls = glm::translate(glm::mat4(1.f), { dx,FY + 1.08f,dz - 0.27f });
        ls = glm::rotate(ls, glm::radians(-15.f), { 1.f,0.f,0.f });
        ls = glm::scale(ls, { 0.62f,0.38f,0.03f });
        drawCube(sh, ls, sc);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    }
    // Desk lamp
    deskLamp(sh, dx + 0.52f, FY + 0.83f, dz - 0.22f);
    // Chair
    studyChair(sh, dx, FY, dz + 0.70f);

    // ── WARDROBE ───────────────────────────────────────────────────────────
    float wx = mirrorX ? (roomX0 + 0.82f) : (roomX1 - 0.82f);
    float wz = roomZ1 - 0.46f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { wx,FY + 1.25f,wz }), { 1.52f,2.50f,0.70f });
    drawCube(sh, m, WOOD_L, texWood, 2.f);
    // Top panel
    m = glm::scale(glm::translate(glm::mat4(1.f), { wx,FY + 2.52f,wz }), { 1.64f,0.12f,0.78f });
    drawCube(sh, m, WOOD_M, texWood, 1.f);
    // Mirror door panel
    m = glm::scale(glm::translate(glm::mat4(1.f), { wx,FY + 1.68f,wz - 0.36f }), { 0.56f,1.18f,0.04f });
    drawCrystalGlass(sh, m);
    // Door handles
    for (int h : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { wx + h * 0.30f,FY + 1.28f,wz - 0.38f }), { 0.06f,0.22f,0.06f });
        drawCylinder(sh, m, glm::vec3(0.75f, 0.68f, 0.45f));
    }

    // ── BOOKSHELF (against south wall to one side) ──────────────────────
    float shx = mirrorX ? (roomX1 - 1.30f) : (roomX0 + 1.30f);
    bookShelf(sh, shx, FY, roomZ0 + 0.18f, 0.70f, true);

    // ── MINI FRIDGE ─────────────────────────────────────────────────────
    float fx = mirrorX ? (roomX1 - 0.35f) : (roomX0 + 0.35f);
    float fz = roomZ1 - 0.40f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { fx,FY + 0.55f,fz }), { 0.58f,1.10f,0.56f });
    drawCube(sh, m, glm::vec3(0.82f, 0.82f, 0.84f), texConcrete, 1.f);
    glm::vec3 led(0.10f, 0.85f, 0.20f);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(led));
    m = glm::scale(glm::translate(glm::mat4(1.f), { fx - 0.30f,FY + 0.28f,fz - 0.20f }), { 0.04f,0.05f,0.05f });
    drawCube(sh, m, led);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    // Fridge handle
    m = glm::scale(glm::translate(glm::mat4(1.f), { fx - 0.30f,FY + 0.70f,fz - 0.29f }), { 0.04f,0.28f,0.06f });
    drawCube(sh, m, STEEL);
}

// ─── Internal staircase (inside building) ─────────────────────────────────────
static void internalStaircase(unsigned int sh, float cx, float cz, float FY, float FH, bool flipDir = false) {
    const int   NS = 8;
    const float RISE = FH / NS;
    const float RUN = 0.65f;
    const float SW = 2.60f;  // stair width
    const float DIR = flipDir ? -1.f : 1.f;
    glm::mat4 m;

    for (int s = 0; s < NS; s++) {
        float sy = FY + (s + 1) * RISE;
        float sz = cz + DIR * (s * RUN + RUN * 0.5f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { cx, sy * 0.5f, sz }), { SW,sy,RUN });
        drawCube(sh, m, CONC, texConcrete, 1.5f);
    }

    // Handrails
    for (int side : {-1, 1}) {
        float rx = cx + side * (SW * 0.5f - 0.08f);
        // Bottom post
        m = glm::scale(glm::translate(glm::mat4(1.f), { rx,FY + 0.55f,cz }), { 0.08f,1.10f,0.08f });
        drawCylinder(sh, m, IRON);
        // Top post
        float topZ = cz + DIR * NS * RUN;
        float topY = FY + FH + 0.55f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { rx,topY * 0.5f,topZ }), { 0.08f,topY,0.08f });
        drawCylinder(sh, m, IRON);
        // Angled rail
        float midZ = (cz + topZ) * 0.5f;
        float midY = (FY + 0.55f + topY) * 0.5f;
        float dZ = topZ - cz;
        float dY = topY - (FY + 0.55f);
        float rLen = sqrtf(dZ * dZ + dY * dY);
        float ang = atanf(dY / fabsf(dZ));
        glm::mat4 rail = glm::translate(glm::mat4(1.f), { rx,midY,midZ });
        rail = glm::rotate(rail, flipDir ? ang : -ang, { 1.f,0.f,0.f });
        rail = glm::scale(rail, { 0.08f,0.08f,rLen });
        drawCube(sh, rail, IRON);
    }

    // Top landing
    float topZ = cz + DIR * NS * RUN;
    m = glm::scale(glm::translate(glm::mat4(1.f), { cx,FY + FH + 0.09f,topZ + DIR * 0.6f }), { SW,0.16f,1.20f });
    drawCube(sh, m, CONC, texConcrete, 1.f);
}

// ─── Corridor (inner hallway) ─────────────────────────────────────────────────
static void innerCorridor(unsigned int sh,
    float corrX0, float corrX1,
    float Z0, float Z1,
    float FY, float CLG)
{
    float xc = (corrX0 + corrX1) * 0.5f, zc = (Z0 + Z1) * 0.5f;
    float cw = corrX1 - corrX0, cd = Z1 - Z0;
    glm::mat4 m;
    // Floor
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY - 0.06f,zc }), { cw,0.12f,cd });
    drawCube(sh, m, glm::vec3(0.75f, 0.70f, 0.62f), texTile, 6.f);
    // Ceiling
    m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FY + CLG + 0.06f,zc }), { cw,0.10f,cd });
    drawCube(sh, m, CREAM, texConcrete, 4.f);
    // Corridor lights every 5 units
    int nl = (int)(cd / 5.0f) + 1;
    for (int i = 0; i < nl; i++) {
        float lz = Z0 + 2.5f + i * 5.0f;
        if (lz < Z1 - 1.f) pendantLight(sh, xc, FY + CLG - 0.02f, lz);
    }
    // Wall sconces on both sides of corridor
    for (int i = 0; i < (nl - 1); i++) {
        float lz = Z0 + 5.0f + i * 5.0f;
        if (lz < Z1 - 1.f) {
            wallLight(sh, corrX0 + 0.05f, FY + 1.80f, lz, +1);
            wallLight(sh, corrX1 - 0.05f, FY + 1.80f, lz, -1);
        }
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  LEFT BUILDING – Student Living (2 floors × 4 rooms + corridor + staircases)
// ════════════════════════════════════════════════════════════════════════════
void renderDormBlockA(unsigned int sh) {
    const float X0 = -22.f, X1 = -10.f, XC = (X0 + X1) * 0.5f;
    const float Z0 = -16.f, Z1 = 16.f, ZC = (Z0 + Z1) * 0.5f;
    const float W = X1 - X0, D = Z1 - Z0;
    const float FH = 3.6f, TH = FH * 2.f;
    const float WALL_T = 0.35f;
    glm::mat4 m;

    // ═══════════════════════════════════════════════════════
    //  EXTERIOR SHELL (unchanged)
    // ═══════════════════════════════════════════════════════
    // Ground-floor brick shell
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.25f,ZC }), { W,FH * 0.5f,D });
    drawCube(sh, m, BRICK, texBrick, 8.f);
    // Upper concrete shell
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH + FH * 0.5f,ZC }), { W,FH,D });
    drawCube(sh, m, CONC, texConcrete, 8.f);
    // Floor slab
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH,ZC }), { W + 0.1f,0.28f,D + 0.1f });
    drawCube(sh, m, TRIM, texConcrete, 5.f);
    // Flat roof
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,TH + 0.18f,ZC }), { W + 0.8f,0.38f,D + 0.8f });
    drawCube(sh, m, ROOF_C, texConcrete, 6.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,TH + 0.46f,ZC }), { W + 0.82f,0.35f,D + 0.82f });
    drawCube(sh, m, TRIM, texConcrete, 6.f);

    // Exterior ground-floor walls
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.5f,Z1 + WALL_T * 0.5f }), { W,FH,WALL_T });
    drawCube(sh, m, BRICK, texBrick, 3.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.5f,Z0 - WALL_T * 0.5f }), { W,FH,WALL_T });
    drawCube(sh, m, BRICK, texBrick, 3.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { X0 - WALL_T * 0.5f,FH * 0.5f,ZC }), { WALL_T,FH,D });
    drawCube(sh, m, BRICK, texBrick, 5.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { X1 + WALL_T * 0.5f,FH * 0.5f,ZC }), { WALL_T,FH,D });
    drawCube(sh, m, BRICK, texBrick, 5.f);
    // Main entrance door (courtyard/east face)
    m = glm::scale(glm::translate(glm::mat4(1.f), { X1 + WALL_T * 0.5f,FH * 0.40f,ZC }), { WALL_T + 0.04f,FH * 0.78f,2.20f });
    drawCube(sh, m, WOOD_D, texWood, 1.f);

    // Courtyard-facing windows GF
    for (int i = 0; i < 3; i++) {
        float wz = Z0 + 4.f + i * 7.2f;
        if (fabsf(wz - ZC) < 1.5f) continue;
        m = glm::scale(glm::translate(glm::mat4(1.f), { X1 + 0.01f,FH * 0.56f,wz }), { WALL_T + 0.04f,1.80f,2.40f });
        drawCrystalGlass(sh, m);
        m = glm::scale(glm::translate(glm::mat4(1.f), { X1 + 0.01f,FH * 0.56f + 0.98f,wz }), { WALL_T + 0.06f,0.22f,2.60f });
        drawCube(sh, m, TRIM, texConcrete, 1.f);
        wallLight(sh, X1 + 0.04f, FH * 0.72f, wz, +1);
    }
    // Outer face small windows GF
    for (int i = 0; i < 4; i++) {
        float wz = Z0 + 4.f + i * 7.5f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { X0 + 0.01f,FH * 0.56f,wz }), { WALL_T + 0.04f,1.60f,2.0f });
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

    // ═══════════════════════════════════════════════════════
    //  INTERIOR LAYOUT
    //  Corridor runs east side (X1 side) along full depth
    //  Rooms are west of corridor, 4 per floor
    //
    //  x range:  X0+WALL_T  →  corrX0   = rooms
    //            corrX0     →  X1-WALL_T = corridor (width ~2.0)
    //
    //  z slots (4 rooms equally spaced):
    //    Room 0: Z0+WALL_T .. Z0+8.0
    //    Room 1: Z0+8.0    .. ZC
    //    Room 2: ZC        .. Z1-8.0
    //    Room 3: Z1-8.0    .. Z1-WALL_T
    // ═══════════════════════════════════════════════════════
    const float CORR_W = 2.0f;
    const float corrX0 = X1 - WALL_T - CORR_W;  // ≈ -12.35
    const float corrX1 = X1 - WALL_T;            // ≈ -10.35
    const float ROOM_X0 = X0 + WALL_T;
    const float ROOM_X1 = corrX0;

    // Room Z boundaries (4 rooms)
    const float RZB[5] = {
        Z0 + WALL_T,
        Z0 + 8.0f,
        ZC,
        Z1 - 8.0f,
        Z1 - WALL_T
    };

    // ── Ground floor rooms ────────────────────────────────────────────────
    {
        const float FY = 0.62f, CLG = FH - 0.52f;
        for (int r = 0; r < 4; r++) {
            bool flip = (r % 2 == 1);
            dormRoom(sh, ROOM_X0, ROOM_X1, RZB[r], RZB[r + 1], FY, CLG, r + 101, flip);
        }
        // Ground floor corridor
        innerCorridor(sh, corrX0, corrX1, Z0, Z1, FY, CLG);
    }

    // ── Upper floor rooms ─────────────────────────────────────────────────
    {
        const float FY = FH + 0.62f, CLG = FH - 0.52f;
        for (int r = 0; r < 4; r++) {
            bool flip = (r % 2 == 0);
            dormRoom(sh, ROOM_X0, ROOM_X1, RZB[r], RZB[r + 1], FY, CLG, r + 201, flip);
        }
        // Upper floor corridor
        innerCorridor(sh, corrX0, corrX1, Z0, Z1, FY, CLG);
    }

    // ── Internal staircases (2 total, flanking the corridor) ─────────────
    // Staircase 1 (north end, ascending south→north)
    {
        float scX = corrX0 - 1.50f;
        float scZ = Z0 + WALL_T + 0.5f;
        internalStaircase(sh, scX, scZ, 0.62f, FH, false);
    }
    // Staircase 2 (south end, ascending north→south i.e. flipped)
    {
        float scX = corrX0 - 1.50f;
        float scZ = Z1 - WALL_T - 0.5f;
        internalStaircase(sh, scX, scZ, 0.62f, FH, true);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  RIGHT BUILDING – Common Facilities
//    Ground floor:
//      West half  (X0..XC):  Reading Room
//      East half  (XC..X1):  Common/TV Room
//    Upper floor:
//      Full width (X0..X1):  Dining Hall + Serving Counter
// ════════════════════════════════════════════════════════════════════════════
void renderDormBlockB(unsigned int sh) {
    const float X0 = 10.f, X1 = 22.f, XC = (X0 + X1) * 0.5f;
    const float Z0 = -16.f, Z1 = 16.f, ZC = (Z0 + Z1) * 0.5f;
    const float W = X1 - X0, D = Z1 - Z0;
    const float FH = 3.6f, TH = FH * 2.f;
    const float WALL_T = 0.35f;
    glm::mat4 m;

    // ═══════════════════════════════════════════════════════
    //  EXTERIOR SHELL (unchanged)
    // ═══════════════════════════════════════════════════════
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.25f,ZC }), { W,FH * 0.5f,D });
    drawCube(sh, m, BRICK, texBrick, 8.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH + FH * 0.5f,ZC }), { W,FH,D });
    drawCube(sh, m, CONC, texConcrete, 8.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH,ZC }), { W + 0.1f,0.28f,D + 0.1f });
    drawCube(sh, m, TRIM, texConcrete, 5.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,TH + 0.18f,ZC }), { W + 0.8f,0.38f,D + 0.8f });
    drawCube(sh, m, ROOF_C, texConcrete, 6.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,TH + 0.46f,ZC }), { W + 0.82f,0.35f,D + 0.82f });
    drawCube(sh, m, TRIM, texConcrete, 6.f);

    // Ground floor full walls
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.5f,Z1 + WALL_T * 0.5f }), { W,FH,WALL_T }); drawCube(sh, m, BRICK, texBrick, 3.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FH * 0.5f,Z0 - WALL_T * 0.5f }), { W,FH,WALL_T }); drawCube(sh, m, BRICK, texBrick, 3.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { X1 + WALL_T * 0.5f,FH * 0.5f,ZC }), { WALL_T,FH,D }); drawCube(sh, m, BRICK, texBrick, 5.f);
    // West courtyard wall with door
    m = glm::scale(glm::translate(glm::mat4(1.f), { X0 - WALL_T * 0.5f,FH * 0.5f,ZC }), { WALL_T,FH,D }); drawCube(sh, m, BRICK, texBrick, 5.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { X0 - WALL_T * 0.5f,FH * 0.40f,ZC }), { WALL_T + 0.04f,FH * 0.78f,2.20f }); drawCube(sh, m, WOOD_D, texWood, 1.f);

    // Courtyard-facing windows (both floors)
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
        for (int i = 0; i < 4; i++) {
            float wz = Z0 + 4.f + i * 7.5f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { X1 - 0.01f,fy + FH * 0.56f,wz }), { WALL_T + 0.04f,1.60f,2.0f }); drawGlass(sh, m);
        }
    }
    drainPipe(sh, X1 + 0.04f, Z0 + 2.f, TH + 0.5f);
    drainPipe(sh, X1 + 0.04f, Z0 + 16.f, TH + 0.5f);

    // ═══════════════════════════════════════════════════════
    //  GROUND FLOOR INTERIOR
    //  West half = READING ROOM  |  East half = COMMON ROOM
    // ═══════════════════════════════════════════════════════
    {
        const float FY = 0.62f, CLG = FH - 0.52f;
        const float RRX0 = X0 + WALL_T, RRX1 = XC - 0.15f;  // Reading Room
        const float CRX0 = XC + 0.15f, CRX1 = X1 - WALL_T; // Common Room
        const float RRXC = (RRX0 + RRX1) * 0.5f;
        const float CRXC = (CRX0 + CRX1) * 0.5f;

        // ── Dividing wall with doorway ────────────────────────────────────
        float dw = 1.60f, dh = 2.20f;
        // Left of doorway
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY + CLG * 0.5f,ZC - dw * 0.5f - ((D - dw) * 0.25f) }), { WALL_T * 0.8f,CLG,(D - dw) * 0.5f });
        drawCube(sh, m, CREAM, texConcrete, 2.f);
        // Right of doorway
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY + CLG * 0.5f,ZC + dw * 0.5f + ((D - dw) * 0.25f) }), { WALL_T * 0.8f,CLG,(D - dw) * 0.5f });
        drawCube(sh, m, CREAM, texConcrete, 2.f);
        // Above doorway
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY + dh + (CLG - dh) * 0.5f,ZC }), { WALL_T * 0.8f,CLG - dh,dw });
        drawCube(sh, m, CREAM, texConcrete, 1.f);
        // Divider door (open)
        {
            glm::mat4 db = glm::translate(glm::mat4(1.f), { XC,FY + dh * 0.5f,ZC - dw * 0.5f });
            db = glm::rotate(db, glm::radians(80.f), { 0,1,0 });
            m = glm::scale(db, { dw,dh,WALL_T + 0.02f });
            drawCube(sh, m, WOOD_D, texWood, 1.f);
        }

        // ── READING ROOM ──────────────────────────────────────────────────
        // Floor
        m = glm::scale(glm::translate(glm::mat4(1.f), { RRXC,FY - 0.06f,ZC }), { RRX1 - RRX0,0.12f,D });
        drawCube(sh, m, glm::vec3(0.74f, 0.68f, 0.58f), texWood, 6.f);
        // Ceiling
        m = glm::scale(glm::translate(glm::mat4(1.f), { RRXC,FY + CLG + 0.06f,ZC }), { RRX1 - RRX0,0.10f,D });
        drawCube(sh, m, CREAM, texConcrete, 4.f);
        // Lights (2 rows of 3)
        for (int i = 0; i < 3; i++) {
            float lz = Z0 + 4.5f + i * 6.5f;
            pendantLight(sh, RRXC - 1.2f, FY + CLG - 0.02f, lz);
            pendantLight(sh, RRXC + 1.2f, FY + CLG - 0.02f, lz);
        }

        // Bookshelves lining west and north/south walls
        for (int i = 0; i < 3; i++) {
            float bsz = Z0 + WALL_T + 0.5f + i * 9.5f;
            bookShelf(sh, X0 + WALL_T + 0.2f, FY, bsz, 0.75f, false); // west wall
        }
        // Back (east) wall sign board
        m = glm::scale(glm::translate(glm::mat4(1.f), { RRX1 - 0.20f,FY + 2.0f,ZC }), { 0.10f,0.60f,3.80f });
        drawCube(sh, m, glm::vec3(0.20f, 0.35f, 0.55f));  // blue noticeboard
        // "READING ROOM" label blocks
        for (int b = 0; b < 5; b++) {
            glm::vec3 lc(0.95f, 0.92f, 0.80f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(lc * 0.30f));
            m = glm::scale(glm::translate(glm::mat4(1.f), { RRX1 - 0.22f,FY + 2.0f,ZC - 1.6f + b * 0.80f }), { 0.08f,0.28f,0.55f });
            drawCube(sh, m, lc);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
        }

        // Study desks (2 rows, 3 tables each)
        for (int row = 0; row < 2; row++) {
            float dz2 = ZC - 7.5f + row * 6.8f;
            for (int col = 0; col < 2; col++) {
                float dx2 = RRXC - 1.5f + col * 3.0f;
                // Table top
                m = glm::scale(glm::translate(glm::mat4(1.f), { dx2,FY + 0.76f,dz2 }), { 1.75f,0.07f,0.80f });
                drawCube(sh, m, WOOD_L, texWood, 1.f);
                // Table legs
                for (int lx : {-1, 1}) {
                    m = glm::scale(glm::translate(glm::mat4(1.f), { dx2 + lx * 0.74f,FY + 0.37f,dz2 }), { 0.08f,0.72f,0.08f });
                    drawCylinder(sh, m, WOOD_D);
                }
                // Chair
                studyChair(sh, dx2, FY, dz2 + (row == 0 ? -0.70f : 0.70f));
                // Desk lamp
                deskLamp(sh, dx2 + 0.60f, FY + 0.83f, dz2 - 0.22f);
                // Books on desk
                glm::vec3 bkc2[4] = { BOOK_RED,BOOK_BLU,BOOK_GRN,BOOK_YEL };
                for (int bk = 0; bk < 3; bk++) {
                    m = glm::scale(glm::translate(glm::mat4(1.f), { dx2 - 0.50f + bk * 0.22f,FY + 0.80f,dz2 - 0.30f }), { 0.08f,0.22f,0.20f });
                    drawCube(sh, m, bkc2[bk]);
                }
            }
        }

        // ── COMMON ROOM ───────────────────────────────────────────────────
        // Floor
        m = glm::scale(glm::translate(glm::mat4(1.f), { CRXC,FY - 0.06f,ZC }), { CRX1 - CRX0,0.12f,D });
        drawCube(sh, m, FLOOR_C, texTile, 8.f);
        // Ceiling
        m = glm::scale(glm::translate(glm::mat4(1.f), { CRXC,FY + CLG + 0.06f,ZC }), { CRX1 - CRX0,0.10f,D });
        drawCube(sh, m, CREAM, texConcrete, 4.f);
        // Lights
        pendantLight(sh, CRXC, FY + CLG - 0.02f, ZC - 4.f);
        pendantLight(sh, CRXC, FY + CLG - 0.02f, ZC + 4.f);

        // Sofa set (L-shape)
        float sx = CRX0 + 0.65f, szSofa = ZC - 1.8f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { sx,FY + 0.46f,szSofa }), { 0.80f,0.46f,2.60f }); drawCube(sh, m, SOFA_C);
        m = glm::scale(glm::translate(glm::mat4(1.f), { sx - 0.38f,FY + 0.72f,szSofa }), { 0.10f,0.50f,2.60f }); drawCube(sh, m, SOFA_C);
        szSofa = ZC + 2.6f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { CRXC + 0.5f,FY + 0.46f,szSofa }), { 2.0f,0.46f,0.80f }); drawCube(sh, m, SOFA_C);
        m = glm::scale(glm::translate(glm::mat4(1.f), { CRXC + 0.5f,FY + 0.72f,szSofa + 0.38f }), { 2.0f,0.50f,0.10f }); drawCube(sh, m, SOFA_C);
        // Coffee table
        m = glm::scale(glm::translate(glm::mat4(1.f), { CRXC + 0.30f,FY + 0.34f,ZC + 0.5f }), { 1.20f,0.08f,1.20f }); drawCube(sh, m, WOOD_M, texWood, 1.f);
        // TV wall mount
        {
            float tvX = CRX1 - 0.16f, tvY = FY + 1.60f, tvZ = ZC - 5.0f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { tvX,tvY,tvZ }), { 0.22f,1.68f,2.78f }); drawCube(sh, m, glm::vec3(0.08f, 0.08f, 0.09f));
            float t2 = (float)glfwGetTime(), bv = 0.25f + 0.10f * sinf(t2 * 0.5f);
            glm::vec3 scrC = { bv * 0.20f,bv * 0.45f,bv * 0.95f };
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(scrC * 0.70f));
            m = glm::scale(glm::translate(glm::mat4(1.f), { tvX - 0.02f,tvY,tvZ }), { 0.06f,1.50f,2.60f }); drawCube(sh, m, scrC);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
        }
        // Indoor plant corner
        {
            float px2 = CRX0 + 0.40f, pz2 = Z1 - WALL_T - 0.55f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { px2,FY + 0.22f,pz2 }), { 0.50f,0.44f,0.50f }); drawCylinder(sh, m, BRICK, texBrick, 1.f);
            m = glm::scale(glm::translate(glm::mat4(1.f), { px2,FY + 0.55f,pz2 }), { 0.08f,0.55f,0.08f }); drawCylinder(sh, m, COL_TRUNK);
            m = glm::scale(glm::translate(glm::mat4(1.f), { px2,FY + 1.10f,pz2 }), { 0.80f,0.80f,0.80f }); drawSphere(sh, m, PLANT, texGrass, 1.f);
        }

        // Staircase up to first floor (inside right building, east end)
        internalStaircase(sh, CRX1 - 1.20f, Z0 + WALL_T + 0.5f, FY, FH, false);
    }

    // ═══════════════════════════════════════════════════════
    //  UPPER FLOOR – DINING HALL
    // ═══════════════════════════════════════════════════════
    {
        const float FY = FH + 0.62f, CLG = FH - 0.52f;
        // Floor
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY - 0.06f,ZC }), { W,0.10f,D });
        drawCube(sh, m, glm::vec3(0.78f, 0.73f, 0.62f), texTile, 8.f);
        // Ceiling
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY + CLG + 0.06f,ZC }), { W,0.10f,D });
        drawCube(sh, m, CREAM, texConcrete, 5.f);
        // Pendant lights (4 rows of 2)
        for (int i = 0; i < 4; i++) {
            float pz = Z0 + 4.0f + i * 7.5f;
            pendantLight(sh, XC - 1.8f, FY + CLG - 0.02f, pz);
            pendantLight(sh, XC + 1.8f, FY + CLG - 0.02f, pz);
        }

        // ── Dining tables (3 rows, 2 tables per row) ──────────────────────
        for (int row = 0; row < 4; row++) {
            float tz = Z0 + 3.5f + row * 7.8f;
            for (int col = 0; col < 2; col++) {
                float tx = XC - 2.8f + col * 5.6f;
                // Table top
                m = glm::scale(glm::translate(glm::mat4(1.f), { tx,FY + 0.76f,tz }), { 2.40f,0.08f,1.20f });
                drawCube(sh, m, WOOD_L, texWood, 1.f);
                // Table legs
                for (int lx : {-1, 1}) for (int lz2 : {-1, 1}) {
                    m = glm::scale(glm::translate(glm::mat4(1.f), { tx + lx * 1.02f,FY + 0.37f,tz + lz2 * 0.46f }), { 0.09f,0.74f,0.09f });
                    drawCylinder(sh, m, WOOD_D);
                }
                // Chairs on both long sides (2 per side)
                for (int side : {-1, 1}) for (int ci = 0; ci < 2; ci++) {
                    float cz2 = tz + (ci == 0 ? -0.88f : 0.88f);
                    studyChair(sh, tx, FY, cz2, side * 90.f);
                }
                // Plate + cup settings (4 settings per table)
                for (int s = 0; s < 4; s++) {
                    float px3 = tx - 1.0f + (s % 2) * 2.0f;
                    float pz3 = tz + (s < 2 ? -0.50f : 0.50f);
                    // Plate
                    m = glm::scale(glm::translate(glm::mat4(1.f), { px3,FY + 0.80f,pz3 }), { 0.28f,0.03f,0.28f });
                    drawCylinder(sh, m, WHITE);
                    // Cup
                    m = glm::scale(glm::translate(glm::mat4(1.f), { px3 + 0.20f,FY + 0.86f,pz3 + 0.15f }), { 0.10f,0.14f,0.10f });
                    drawCylinder(sh, m, CREAM);
                }
            }
        }

        // ── Serving counter (back wall z=Z1 side) ─────────────────────────
        float kz = Z1 - WALL_T - 0.62f;
        // Counter base
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY + 0.50f,kz }), { W - 1.0f,0.80f,0.75f });
        drawCube(sh, m, COUNTER, texConcrete, 3.f);
        // Counter top (marble look)
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY + 0.92f,kz }), { W - 0.8f,0.10f,0.82f });
        drawCube(sh, m, glm::vec3(0.90f, 0.88f, 0.85f), texMarble, 2.f);
        // Counter front panel decorative strip
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY + 0.50f,kz - 0.38f }), { W - 1.0f,0.80f,0.06f });
        drawCube(sh, m, BRICK, texBrick, 4.f);
        // Food trays on counter
        for (int t = 0; t < 5; t++) {
            float tx2 = X0 + WALL_T + 0.8f + t * 2.0f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { tx2,FY + 0.98f,kz - 0.10f }), { 1.60f,0.06f,0.55f });
            drawCube(sh, m, STEEL);
            // Food dome
            m = glm::scale(glm::translate(glm::mat4(1.f), { tx2,FY + 1.06f,kz - 0.10f }), { 1.50f,0.15f,0.50f });
            glm::vec3 foodCol(0.7f + t * 0.05f, 0.5f - t * 0.03f, 0.3f);
            drawCube(sh, m, foodCol);
        }
        // Counter lamps / overhead hanging lights
        for (int i = 0; i < 3; i++) {
            float lx = X0 + WALL_T + 1.5f + i * 3.5f;
            pendantLight(sh, lx, FY + CLG - 0.02f, kz - 0.20f);
        }
        // Signage above counter
        m = glm::scale(glm::translate(glm::mat4(1.f), { XC,FY + CLG - 0.40f,Z1 - 0.10f }), { W - 1.0f,0.48f,0.10f });
        drawCube(sh, m, glm::vec3(0.15f, 0.30f, 0.15f));  // green board
        // Sign lettering
        for (int b = 0; b < 6; b++) {
            glm::vec3 wlc(0.95f, 0.92f, 0.75f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(wlc * 0.40f));
            m = glm::scale(glm::translate(glm::mat4(1.f), { X0 + WALL_T + 0.6f + b * 1.85f,FY + CLG - 0.40f,Z1 - 0.12f }), { 1.20f,0.34f,0.06f });
            drawCube(sh, m, wlc);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
        }

        // ── Upper floor staircase landing area ────────────────────────────
        // Landing platform at top of the internal staircase
        {
            float lx = X1 - WALL_T - 1.20f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { lx,FY - 0.05f,Z0 + WALL_T + 0.5f + (float)8 * 0.65f + 0.8f }), { 2.40f,0.12f,1.80f });
            drawCube(sh, m, CONC, texConcrete, 1.f);
            // Railing at landing edge
            m = glm::scale(glm::translate(glm::mat4(1.f), { lx,FY + 0.55f,Z0 + WALL_T + 6.5f }), { 2.40f,1.10f,0.09f });
            drawCube(sh, m, IRON);
        }
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  CORRIDOR (covered external walkways between buildings)
// ════════════════════════════════════════════════════════════════════════════
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

// ════════════════════════════════════════════════════════════════════════════
//  COURTYARD
// ════════════════════════════════════════════════════════════════════════════
void renderCourtyard(unsigned int sh) {
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f,0.01f,0.f }), { 19.0f,0.08f,31.0f }); drawCube(sh, m, COL_GRASS, texGrass, 6.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f,0.04f,7.0f }), { 4.0f,0.06f,42.0f }); drawCube(sh, m, COL_PLAZA, texTile, 8.f);
    for (int s : {-1, 1}) for (int i = 0; i < 4; i++) decorTree(sh, s * 4.5f, -10.f + i * 6.5f);

    auto bench = [&](float x, float z, float rotY) {
        glm::mat4 base = glm::rotate(glm::translate(glm::mat4(1.f), { x,0.72f,z }), glm::radians(rotY), { 0,1,0 });
        drawCube(sh, glm::scale(glm::translate(base, { 0.f,0.f,0.f }), { 2.0f,0.11f,0.50f }), WOOD_L, texWood, 1.f);
        for (int s : {-1, 1}) { drawCube(sh, glm::scale(glm::translate(base, { s * 0.80f,-0.30f,0.18f }), { 0.10f,0.60f,0.10f }), TRIM); drawCube(sh, glm::scale(glm::translate(base, { s * 0.80f,-0.30f,-0.18f }), { 0.10f,0.60f,0.10f }), TRIM); }
        drawCube(sh, glm::scale(glm::translate(base, { 0.f,0.38f,-0.22f }), { 2.0f,0.44f,0.08f }), WOOD_L, texWood, 1.f);
        };
    for (int i = 0; i < 3; i++) { bench(-3.0f, -8.0f + i * 6.5f, 15.f); bench(3.0f, -8.0f + i * 6.5f, -15.f); }

    // Central staircase (unchanged)
    {
        const float SX = 0.f, SZ_FRONT = -10.0f, SWIDTH = 10.0f;
        const int NSTEPS = 9;
        const float RISE = 0.44f, RUN = 0.70f;
        const float TOP_Y = NSTEPS * RISE;
        const float SZ_TOP = SZ_FRONT - NSTEPS * RUN;
        glm::mat4 m2;
        for (int step = 0; step < NSTEPS; step++) {
            float sy = (step + 1) * RISE, sz = SZ_FRONT - step * RUN - RUN * 0.5f;
            m2 = glm::scale(glm::translate(glm::mat4(1.f), { SX,sy * 0.5f,sz }), { SWIDTH,sy,RUN });
            drawCube(sh, m2, CONC, texConcrete, 2.f);
        }
        for (int s : {-1, 1}) {
            float rx = SX + s * (SWIDTH * 0.5f - 0.10f);
            m2 = glm::scale(glm::translate(glm::mat4(1.f), { rx,0.55f,SZ_FRONT }), { 0.10f,1.10f,0.10f }); drawCylinder(sh, m2, IRON);
            float postTopY = TOP_Y + 0.55f;
            m2 = glm::scale(glm::translate(glm::mat4(1.f), { rx,postTopY * 0.5f,SZ_TOP }), { 0.10f,postTopY,0.10f }); drawCylinder(sh, m2, IRON);
            float midZ = (SZ_FRONT + SZ_TOP) * 0.5f, midY = (0.55f + postTopY) * 0.5f;
            float dZ = SZ_TOP - SZ_FRONT, dY = postTopY - 0.55f;
            float rLen = sqrtf(dZ * dZ + dY * dY), ang = atanf(dY / fabsf(dZ));
            glm::mat4 rail = glm::translate(glm::mat4(1.f), { rx,midY,midZ });
            rail = glm::rotate(rail, ang, { 1.f,0.f,0.f }); rail = glm::scale(rail, { 0.09f,0.09f,rLen });
            drawCube(sh, rail, IRON);
            for (int b = 1; b < NSTEPS; b++) {
                float bz = SZ_FRONT - b * RUN, by = b * RISE + 0.55f;
                m2 = glm::scale(glm::translate(glm::mat4(1.f), { rx,by * 0.5f,bz }), { 0.07f,by,0.07f }); drawCylinder(sh, m2, IRON);
            }
        }
        const float LAND_DEPTH = 1.0f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { SX,TOP_Y + 0.09f,SZ_TOP + LAND_DEPTH * 0.5f }), { SWIDTH,0.18f,LAND_DEPTH }); drawCube(sh, m, CONC, texConcrete, 2.f);
        const float WING_Z = SZ_TOP + LAND_DEPTH;
        const float WING_W_L = (SWIDTH * 0.5f) + (10.f - SWIDTH * 0.5f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { -(WING_W_L * 0.5f),TOP_Y + 0.09f,WING_Z }), { WING_W_L,0.18f,1.80f }); drawCube(sh, m, CONC, texConcrete, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { +(WING_W_L * 0.5f),TOP_Y + 0.09f,WING_Z }), { WING_W_L,0.18f,1.80f }); drawCube(sh, m, CONC, texConcrete, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { SX,TOP_Y + 0.56f,SZ_TOP }), { SWIDTH,0.12f,0.10f }); drawCube(sh, m, IRON);
        m = glm::scale(glm::translate(glm::mat4(1.f), { -10.f,TOP_Y + 0.56f,WING_Z + 0.90f }), { 0.10f,1.12f,1.80f }); drawCube(sh, m, IRON);
        m = glm::scale(glm::translate(glm::mat4(1.f), { +10.f,TOP_Y + 0.56f,WING_Z + 0.90f }), { 0.10f,1.12f,1.80f }); drawCube(sh, m, IRON);
        for (int p = -2; p <= 2; p++) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { SX + p * (SWIDTH / 5.f),TOP_Y + 0.62f,SZ_TOP }), { 0.10f,1.24f,0.10f }); drawCylinder(sh, m, IRON);
        }
    }
}

// ─── Admin annex ──────────────────────────────────────────────────────────────
void renderAdminAnnex(unsigned int sh) {
    const float AX = 20.f, AZ = -20.f;
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX,1.80f,AZ }), { 5.0f,3.60f,5.0f }); drawCube(sh, m, BRICK, texBrick, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX,3.72f,AZ }), { 5.2f,0.28f,5.2f }); drawCube(sh, m, ROOF_C, texConcrete, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX,1.20f,AZ + 2.52f }), { 1.50f,2.40f,0.22f }); drawCube(sh, m, WOOD_D, texWood, 1.f);
}

// ─── Entrance sphere finials ──────────────────────────────────────────────────
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

// ─── Stubs for API compatibility ──────────────────────────────────────────────
void renderRoomInterior(unsigned int sh) {}
void renderDormRoom(unsigned int sh) {}