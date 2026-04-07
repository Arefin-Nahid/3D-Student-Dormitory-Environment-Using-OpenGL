#include "scene_props.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>

static const glm::vec3 ZERO3(0.f, 0.f, 0.f);
static const glm::vec3 IRON(0.20f, 0.20f, 0.22f);
static const glm::vec3 BRICK2(0.62f, 0.28f, 0.18f);
static const glm::vec3 CONC2(0.80f, 0.80f, 0.78f);

// ── Palm tree ────────────────────────────────────────────────────────────────
void renderPalmTree(unsigned int sh, glm::vec3 base) {
    for (int i = 0; i < 5; i++) {
        float y = base.y + 0.5f + i, sc = 0.20f - i * 0.015f;
        glm::mat4 m = glm::translate(glm::mat4(1.f), { base.x, y, base.z });
        m = glm::rotate(m, glm::radians((float)(i * 4)), { 0,1,0 });
        m = glm::scale(m, { sc, 1.f, sc });
        m = glm::rotate(m, glm::radians(90.f), { 1,0,0 });
        drawCylinder(sh, m, COL_TRUNK, texWood, 1.f);
    }
    for (int i = 0; i < 6; i++) {
        glm::mat4 m = glm::translate(glm::mat4(1.f), { base.x, base.y + 5.5f, base.z });
        m = glm::rotate(m, glm::radians(i * 60.f), { 0,1,0 });
        m = glm::translate(m, { 1.2f, 0.3f, 0.f });
        m = glm::rotate(m, glm::radians(-30.f), { 0,0,1 });
        m = glm::scale(m, { 2.5f, 0.12f, 0.5f });
        drawCube(sh, m, COL_PALM, texGrass, 1.f);
    }
}

// ── Cone tree ────────────────────────────────────────────────────────────────
void renderConeTree(unsigned int sh, float x, float z) {
    glm::mat4 m = glm::translate(glm::mat4(1.f), { x, 1.5f, z });
    m = glm::rotate(m, glm::radians(90.f), { 1,0,0 });
    m = glm::scale(m, { 0.4f, 0.4f, 3.f });
    drawCylinder(sh, m, COL_TRUNK, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x, 4.5f, z }), { 3.5f, 4.5f, 3.5f });
    drawCone(sh, m, COL_PALM, texGrass, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x, 7.f, z }), { 2.3f, 3.5f, 2.3f });
    drawCone(sh, m, COL_PALM, texGrass, 1.5f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x, 9.f, z }), { 1.2f, 2.5f, 1.2f });
    drawCone(sh, m, COL_PALM, texGrass, 1.f);
}

// ════════════════════════════════════════════════════════════════════════════
//  FRACTAL TREE  –  recursive branching tree using cylinders
//  depth: recursion level (start=5)
//  Each branch splits into 3 children, slightly shorter and rotated
// ════════════════════════════════════════════════════════════════════════════
static void fractalBranch(unsigned int sh, glm::mat4 base,
    float length, float radius, int depth) {
    if (depth == 0 || length < 0.12f) return;

    // Draw this branch segment as a cylinder
    glm::mat4 cyl = glm::translate(base, { 0.f, length * 0.5f, 0.f });
    cyl = glm::scale(cyl, { radius, length, radius });
    glm::vec3 col = (depth <= 2)
        ? glm::vec3(0.15f + depth * 0.05f, 0.50f + depth * 0.06f, 0.12f) // leaf green
        : glm::vec3(0.38f + (5 - depth) * 0.04f, 0.24f + (5 - depth) * 0.02f, 0.10f); // brown bark
    drawCylinder(sh, cyl, col, (depth <= 2) ? texGrass : texWood, 1.f);

    // Tip of this branch
    glm::mat4 tip = glm::translate(base, { 0.f, length, 0.f });

    float childLen = length * 0.68f;
    float childRadius = radius * 0.62f;

    // 3 child branches at different yaw + lean angles
    float leans[3] = { 28.f, 28.f, 28.f };
    float yaws[3] = { 0.f, 120.f, 240.f };
    for (int i = 0; i < 3; i++) {
        glm::mat4 child = tip;
        child = glm::rotate(child, glm::radians(yaws[i]), { 0.f,1.f,0.f });
        child = glm::rotate(child, glm::radians(leans[i]), { 0.f,0.f,1.f });
        fractalBranch(sh, child, childLen, childRadius, depth - 1);
    }
}

void renderFractalTree(unsigned int sh, float x, float z) {
    // Trunk base transform: translate to position, point upward
    glm::mat4 base = glm::translate(glm::mat4(1.f), { x, 0.0f, z });
    fractalBranch(sh, base, 3.2f, 0.18f, 5);
}

// ── Bench ────────────────────────────────────────────────────────────────────
void renderBench(unsigned int sh, glm::vec3 pos, float rotY) {
    glm::mat4 base = glm::rotate(glm::translate(glm::mat4(1.f), pos), glm::radians(rotY), { 0,1,0 });
    drawCube(sh, glm::scale(glm::translate(base, { 0.f,0.66f,0.f }), { 2.0f,0.12f,0.50f }), COL_TRUNK, texWood, 1.f);
    for (int s : {-1, 1}) {
        drawCube(sh, glm::scale(glm::translate(base, { s * 0.80f,0.33f, 0.18f }), { 0.10f,0.66f,0.10f }), COL_STONE);
        drawCube(sh, glm::scale(glm::translate(base, { s * 0.80f,0.33f,-0.18f }), { 0.10f,0.66f,0.10f }), COL_STONE);
    }
    drawCube(sh, glm::scale(glm::translate(base, { 0.f,1.1f,-0.22f }), { 2.0f,0.5f,0.08f }), COL_TRUNK, texWood, 1.f);
}

// ── Fountain ──────────────────────────────────────────────────────────────────
void renderFountain(unsigned int sh, glm::vec3 pos) {
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), pos), { 4.5f,0.5f,4.5f });
    drawCylinder(sh, m, COL_MARBLE, texMarble, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { pos.x,pos.y + 0.28f,pos.z }), { 3.6f,0.15f,3.6f });
    drawCube(sh, m, COL_WATER);
    m = glm::scale(glm::translate(glm::mat4(1.f), { pos.x,pos.y + 0.25f,pos.z }), { 0.6f,1.8f,0.6f });
    drawCylinder(sh, m, COL_MARBLE, texMarble, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { pos.x,pos.y + 1.8f,pos.z }), { 1.5f,1.5f,1.5f });
    drawSphere(sh, m, COL_MARBLE, texMarble, 2.f);
}

// ── Lamp post ────────────────────────────────────────────────────────────────
void renderLampPost(unsigned int sh, float x, float z) {
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { x, 3.0f, z }), { 0.12f, 6.0f, 0.12f });
    drawCylinder(sh, m, IRON, texConcrete, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x, 6.15f, z }), { 0.36f, 0.50f, 0.36f });
    drawCylinder(sh, m, glm::vec3(0.88f, 0.84f, 0.55f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { x, 6.44f, z }), { 0.42f, 0.12f, 0.42f });
    drawCylinder(sh, m, IRON);
    if (!dayMode) {
        glm::vec3 em(0.92f, 0.85f, 0.48f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(em));
        m = glm::scale(glm::translate(glm::mat4(1.f), { x, 6.15f, z }), { 0.28f, 0.40f, 0.28f });
        drawSphere(sh, m, glm::vec3(1.f, 0.96f, 0.80f), texMarble, 1.f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  WALKING STUDENTS  –  4 simple human figures walking back and forth
//  in front of the dormitory (z = 20..26, x = -18..18)
// ════════════════════════════════════════════════════════════════════════════
static void drawStudent(unsigned int sh, float px, float pz,
    float facingYaw,
    const glm::vec3& shirtCol,
    const glm::vec3& pantsCol,
    float legSwing) {
    const glm::vec3 SKIN(0.88f, 0.70f, 0.55f);
    const glm::vec3 SHOE(0.15f, 0.12f, 0.10f);
    const float BY = 0.04f;
    glm::mat4 m;

    // Shoes
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { px + s * 0.16f, BY + 0.11f, pz }),
            { 0.22f, 0.22f, 0.40f });
        drawCube(sh, m, SHOE);
    }
    // Legs (animated swing)
    {
        glm::mat4 leg = glm::translate(glm::mat4(1.f), { px - 0.14f, BY + 0.62f, pz });
        leg = glm::rotate(leg, glm::radians(legSwing), { 1.f,0.f,0.f });
        m = glm::scale(leg, { 0.22f, 0.78f, 0.22f }); drawCube(sh, m, pantsCol);
    }
    {
        glm::mat4 leg = glm::translate(glm::mat4(1.f), { px + 0.14f, BY + 0.62f, pz });
        leg = glm::rotate(leg, glm::radians(-legSwing), { 1.f,0.f,0.f });
        m = glm::scale(leg, { 0.22f, 0.78f, 0.22f }); drawCube(sh, m, pantsCol);
    }
    // Torso
    m = glm::scale(glm::translate(glm::mat4(1.f), { px, BY + 1.50f, pz }),
        { 0.50f, 0.68f, 0.32f });
    drawCube(sh, m, shirtCol, texConcrete, 1.f);
    // Arms
    for (int s : {-1, 1}) {
        glm::mat4 arm = glm::translate(glm::mat4(1.f), { px + s * 0.34f, BY + 1.55f, pz });
        arm = glm::rotate(arm, glm::radians(s * -legSwing * 0.7f), { 1.f,0.f,0.f });
        m = glm::scale(arm, { 0.14f, 0.52f, 0.14f }); drawCube(sh, m, shirtCol);
    }
    // Head
    m = glm::scale(glm::translate(glm::mat4(1.f), { px, BY + 2.08f, pz }),
        { 0.38f, 0.38f, 0.38f });
    drawSphere(sh, m, SKIN, texMarble, 1.f);
    // Backpack (small cube on back)
    {
        glm::mat4 bp = glm::translate(glm::mat4(1.f), { px, BY + 1.50f, pz });
        bp = glm::rotate(bp, glm::radians(facingYaw), { 0.f,1.f,0.f });
        bp = glm::translate(bp, { 0.f, 0.f, -0.28f });
        bp = glm::scale(bp, { 0.34f, 0.48f, 0.18f });
        drawCube(sh, bp, glm::vec3(0.28f, 0.22f, 0.55f));
    }
}

void renderWalkingStudents(unsigned int sh) {
    float t = (float)glfwGetTime();

    // Student colours
    const glm::vec3 shirts[4] = {
        {0.85f,0.18f,0.18f},  // red shirt
        {0.18f,0.38f,0.80f},  // blue shirt
        {0.20f,0.65f,0.25f},  // green shirt
        {0.90f,0.82f,0.20f},  // yellow shirt
    };
    const glm::vec3 pants[4] = {
        {0.22f,0.22f,0.40f},
        {0.25f,0.18f,0.12f},
        {0.22f,0.22f,0.40f},
        {0.20f,0.20f,0.22f},
    };

    // Each student: different phase, speed, and x offset lane
    struct StudentData {
        float xLane;       // x position (left-right lane)
        float speed;       // z walking speed
        float phase;       // time offset
        float zMin, zMax;  // z walk range
        bool  facesNorth;  // initially walks northward
    };

    StudentData students[4] = {
        { -14.f, 2.8f, 0.0f,  18.f, 26.f, true  },
        {  -6.f, 3.2f, 1.5f,  18.f, 26.f, false },
        {   6.f, 2.6f, 0.8f,  18.f, 26.f, true  },
        {  14.f, 3.0f, 2.2f,  18.f, 26.f, false },
    };

    for (int i = 0; i < 4; i++) {
        StudentData& s = students[i];
        float span = s.zMax - s.zMin;
        float cycle = 2.f * span / s.speed;           // full round-trip time
        float t0 = fmodf(t + s.phase, cycle);
        float pz, yaw;
        bool  goingNorth;

        if (t0 < cycle * 0.5f) {
            // Walking north (z decreasing, into campus)
            pz = s.zMax - s.speed * t0;
            yaw = -90.f;  // faces -z (north)
            goingNorth = true;
        }
        else {
            // Walking south (z increasing, back out)
            pz = s.zMin + s.speed * (t0 - cycle * 0.5f);
            yaw = 90.f;   // faces +z (south)
            goingNorth = false;
        }

        float legSwing = 32.f * sinf(t * 4.5f + s.phase);

        drawStudent(sh, s.xLane, pz, yaw, shirts[i], pants[i], legSwing);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  BEZIER CURVE UTILITIES
//  
//  Cubic Bezier formula: P(t) = (1-t)³·B₀ + 3(1-t)²t·B₁ + 3(1-t)t²·B₂ + t³·B₃
//  where t ∈ [0,1] and B₀, B₁, B₂, B₃ are control points
// ════════════════════════════════════════════════════════════════════════════

// Cubic Bezier curve evaluation for a single parameter value t
static glm::vec3 bezierCubic(const glm::vec3& b0, const glm::vec3& b1,
    const glm::vec3& b2, const glm::vec3& b3, float t) {
    float oneMinusT = 1.0f - t;
    float oneMinusT2 = oneMinusT * oneMinusT;
    float oneMinusT3 = oneMinusT2 * oneMinusT;
    float t2 = t * t;
    float t3 = t2 * t;

    // Bernstein polynomial basis functions
    float b0_coef = oneMinusT3;           // J₃,₀(t) = (1-t)³
    float b1_coef = 3.0f * oneMinusT2 * t; // J₃,₁(t) = 3(1-t)²t
    float b2_coef = 3.0f * oneMinusT * t2; // J₃,₂(t) = 3(1-t)t²
    float b3_coef = t3;                    // J₃,₃(t) = t³

    return b0_coef * b0 + b1_coef * b1 + b2_coef * b2 + b3_coef * b3;
}

// Generate discrete points along Bezier curve
static std::vector<glm::vec3> generateBezierPoints(const glm::vec3& b0, const glm::vec3& b1,
    const glm::vec3& b2, const glm::vec3& b3,
    int numSegments) {
    std::vector<glm::vec3> points;
    points.reserve(numSegments + 1);

    for (int i = 0; i <= numSegments; i++) {
        float t = (float)i / (float)numSegments;
        points.push_back(bezierCubic(b0, b1, b2, b3, t));
    }

    return points;
}

// ════════════════════════════════════════════════════════════════════════════
//  GATE WITH BEZIER CURVE ARCH  –  CYLINDRICAL pillars (unchanged) + curved top
// ════════════════════════════════════════════════════════════════════════════
void renderGate(unsigned int sh) {
    const float GZ = 28.f;
    const float GX = 0.f;
    const float PR = 0.90f;  // pillar radius (cylinder X/Z scale = PR*2)
    const float PHT = 4.80f;  // pillar height
    const float GAP = 4.80f;  // half-gap centre to pillar inner edge

    glm::mat4 m;

    // ── CYLINDRICAL PILLARS (UNCHANGED) ──────────────────────────────────
    for (int s : {-1, 1}) {
        float px = GX + s * (GAP + PR);

        // Plinth base (small cube)
        m = glm::scale(glm::translate(glm::mat4(1.f), { px, 0.28f, GZ }),
            { PR * 2.f + 0.30f, 0.56f, PR * 2.f + 0.30f });
        drawCube(sh, m, CONC2, texConcrete, 1.f);

        // Main cylindrical shaft – uses pillar.png image texture in mode 4
        m = glm::scale(glm::translate(glm::mat4(1.f), { px, PHT * 0.5f + 0.56f, GZ }),
            { PR * 2.f, PHT, PR * 2.f });
        {
            // In texture mode 4 apply the loaded pillar.png; otherwise brick texture
            unsigned int pillarTex = (textureMode == 4 && texPillar != 0) ? texPillar : texBrick;
            drawCylinder(sh, m, BRICK2, pillarTex, 3.f);
        }

        // Capital ring
        m = glm::scale(glm::translate(glm::mat4(1.f), { px, PHT + 0.62f, GZ }),
            { PR * 2.f + 0.24f, 0.40f, PR * 2.f + 0.24f });
        drawCylinder(sh, m, CONC2, texConcrete, 1.f);

        // Sphere finial on top
        m = glm::scale(glm::translate(glm::mat4(1.f), { px, PHT + 0.62f + 0.50f, GZ }),
            { 0.58f, 0.58f, 0.58f });
        drawSphere(sh, m, glm::vec3(0.85f, 0.82f, 0.62f), texMarble, 1.f);

        // Night glow on finial
        if (!dayMode) {
            glm::vec3 em2(0.92f, 0.84f, 0.45f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(em2 * 0.75f));
            m = glm::scale(glm::translate(glm::mat4(1.f), { px, PHT + 0.62f + 0.50f, GZ }),
                { 0.44f, 0.44f, 0.44f });
            drawSphere(sh, m, em2, texMarble, 1.f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
        }
    }

    // ╔═══════════════════════════════════════════════════════════════════════╗
    // ║  BEZIER CURVE ARCH  –  Full 3D extruded structure                    ║
    // ║                                                                       ║
    // ║  MATHEMATICAL MODEL  (Cubic Bezier, n=3):                            ║
    // ║  P(t) = Σ Bᵢ · J₃,ᵢ(t),   t ∈ [0,1]                               ║
    // ║                                                                       ║
    // ║  Bernstein basis polynomials:                                        ║
    // ║    J₃,₀(t) = (1-t)³          → weight of B₀ (left endpoint)         ║
    // ║    J₃,₁(t) = 3(1-t)²t        → weight of B₁ (left control)          ║
    // ║    J₃,₂(t) = 3(1-t)t²        → weight of B₂ (right control)         ║
    // ║    J₃,₃(t) = t³              → weight of B₃ (right endpoint)        ║
    // ║                                                                       ║
    // ║  World-space control points:                                         ║
    // ║    B₀ = (−pillarCentreX,  pillarTopY,  GZ)  ← left pillar top        ║
    // ║    B₁ = (−pillarCentreX,  archPeakY,   GZ)  ← left tension point     ║
    // ║    B₂ = (+pillarCentreX,  archPeakY,   GZ)  ← right tension point    ║
    // ║    B₃ = (+pillarCentreX,  pillarTopY,  GZ)  ← right pillar top       ║
    // ║                                                                       ║
    // ║  3D EXTRUSION METHOD:                                                 ║
    // ║    Two parallel Bezier curves (front face & back face) are sampled   ║
    // ║    at 60 discrete t-values.  Each segment [t_i, t_{i+1}]:            ║
    // ║      • Front beam  – stone/concrete colour                            ║
    // ║      • Back beam   – identical, offset by ARCH_DEPTH in -Z            ║
    // ║      • 4 connecting slab faces (top, bottom, left side, right side)  ║
    // ║    This creates a fully solid, volumetric arch.                       ║
    // ║                                                                       ║
    // ║  OBJECT→WORLD TRANSFORM per segment:                                 ║
    // ║    tangent  = normalise(P(t_{i+1}) − P(t_i))      (along curve)      ║
    // ║    right    = normalise(up × tangent)              (X axis of beam)   ║
    // ║    normal   = tangent × right                      (Y axis of beam)   ║
    // ║    Compose into 4×4 rotation+translation matrix, then scale to       ║
    // ║    (BEAM_W × BEAM_H × segmentLength).                                ║
    // ╚═══════════════════════════════════════════════════════════════════════╝

    {
        // ── Arch geometry parameters ──────────────────────────────────────
        const float PILLAR_TOP_Y  = PHT + 0.62f + 0.20f; // top of capital ring
        const float ARCH_PEAK_Y   = PILLAR_TOP_Y + 2.40f; // peak rises 2.4 above pillars
        const float ARCH_DEPTH    = 0.10f;                 // extrusion depth in Z
        const float BEAM_W        = 0.5f;                 // beam cross-section width
        const float BEAM_H        = 0.5f;                 // beam cross-section height
        const float PILLAR_CX     = GAP + PR;              // x of pillar centre = 5.70

        // ── Bezier control points (world space, front face at z=GZ) ──────
        //  B₀  and  B₃  are endpoints, anchored exactly at pillar tops.
        //  B₁  and  B₂  are interior control points that pull the arch upward.
        //  Setting B₁.x = B₀.x  and  B₂.x = B₃.x  gives a symmetric,
        //  near-parabolic arch profile (control handles are vertical).
        glm::vec3 b0(-PILLAR_CX, PILLAR_TOP_Y, GZ);          // left pillar top
        glm::vec3 b1(-PILLAR_CX * 0.30f, ARCH_PEAK_Y, GZ);   // left tension point
        glm::vec3 b2(+PILLAR_CX * 0.30f, ARCH_PEAK_Y, GZ);   // right tension point
        glm::vec3 b3(+PILLAR_CX, PILLAR_TOP_Y, GZ);          // right pillar top

        // Corresponding back-face control points (offset by -ARCH_DEPTH in Z)
        glm::vec3 b0b(b0.x, b0.y, GZ - ARCH_DEPTH);
        glm::vec3 b1b(b1.x, b1.y, GZ - ARCH_DEPTH);
        glm::vec3 b2b(b2.x, b2.y, GZ - ARCH_DEPTH);
        glm::vec3 b3b(b3.x, b3.y, GZ - ARCH_DEPTH);

        // ── Sample both curves ────────────────────────────────────────────
        const int   N_SEG  = 60;  // number of segments (higher = smoother)
        std::vector<glm::vec3> frontPts = generateBezierPoints(b0, b1, b2, b3, N_SEG);
        std::vector<glm::vec3> backPts  = generateBezierPoints(b0b,b1b,b2b,b3b,N_SEG);

        // Colours / textures
        const glm::vec3 ARCH_COL(0.84f, 0.80f, 0.72f);  // warm cream-stone
        const glm::vec3 SIDE_COL(0.72f, 0.68f, 0.60f);  // slightly darker sides

        // Helper: build a segment box given two endpoints, a width and height
        // along the curve tangent direction, then draw it.
        auto drawBeamSeg = [&](const glm::vec3& pA, const glm::vec3& pB,
                                float bw, float bh,
                                const glm::vec3& col, unsigned int tex, float tile)
        {
            glm::vec3 mid     = (pA + pB) * 0.5f;
            glm::vec3 tangent = glm::normalize(pB - pA);
            glm::vec3 up(0.f, 1.f, 0.f);
            if (fabsf(glm::dot(tangent, up)) > 0.998f) up = glm::vec3(0.f, 0.f, 1.f);

            // Local axes for the beam cross-section
            glm::vec3 right  = glm::normalize(glm::cross(up, tangent));
            glm::vec3 normal = glm::normalize(glm::cross(tangent, right));

            // Object→World rotation matrix (column vectors = local axes)
            glm::mat4 R(1.f);
            R[0] = glm::vec4(right,   0.f);
            R[1] = glm::vec4(normal,  0.f);
            R[2] = glm::vec4(tangent, 0.f);
            R[3] = glm::vec4(mid,     1.f);

            float segLen = glm::length(pB - pA);
            glm::mat4 S  = glm::scale(glm::mat4(1.f), { bw, bh, segLen });
            drawCube(sh, R * S, col, tex, tile);
        };

        // ── Render extruded arch ──────────────────────────────────────────
        for (int i = 0; i < N_SEG; i++) {
            // Front face beam
            drawBeamSeg(frontPts[i], frontPts[i+1],
                        BEAM_W, BEAM_H, ARCH_COL, texConcrete, 1.f);

            // Back face beam
            drawBeamSeg(backPts[i], backPts[i+1],
                        BEAM_W, BEAM_H, ARCH_COL, texConcrete, 1.f);

            // Top connecting slab (joins front and back at the outer/top face)
            glm::vec3 topFront = frontPts[i]   + glm::vec3(0.f, BEAM_H * 0.5f, 0.f);
            glm::vec3 topBack  = backPts[i]    + glm::vec3(0.f, BEAM_H * 0.5f, 0.f);
            glm::vec3 topFrontN= frontPts[i+1] + glm::vec3(0.f, BEAM_H * 0.5f, 0.f);
            glm::vec3 topBackN = backPts[i+1]  + glm::vec3(0.f, BEAM_H * 0.5f, 0.f);
            glm::vec3 topMid   = (topFront + topBack + topFrontN + topBackN) * 0.25f;
            float     segLen   = glm::length(frontPts[i+1] - frontPts[i]);
            // Approximate top slab as a flat cube spanning depth and segment length
            glm::vec3 tangentF = glm::normalize(frontPts[i+1] - frontPts[i]);
            glm::vec3 up2(0.f,1.f,0.f);
            if (fabsf(glm::dot(tangentF,up2))>0.998f) up2={0.f,0.f,1.f};
            glm::vec3 right2  = glm::normalize(glm::cross(up2, tangentF));
            glm::vec3 norm2   = glm::normalize(glm::cross(tangentF, right2));
            glm::mat4 R2(1.f);
            R2[0]=glm::vec4(right2,0.f); R2[1]=glm::vec4(norm2,0.f);
            R2[2]=glm::vec4(tangentF,0.f); R2[3]=glm::vec4(topMid,1.f);
            glm::mat4 S2 = glm::scale(glm::mat4(1.f),{ARCH_DEPTH + BEAM_W, 0.06f, segLen});
            drawCube(sh, R2*S2, SIDE_COL, texConcrete, 1.f);
        }

        // ── Decorative keystone at arch peak ─────────────────────────────
        //   Placed at t=0.5 of the front curve.  Slightly larger block in a
        //   contrasting stone colour to mimic a classical Roman keystone.
        {
            glm::vec3 peakF = bezierCubic(b0, b1, b2, b3, 0.5f);
            glm::vec3 peakB = bezierCubic(b0b,b1b,b2b,b3b, 0.5f);
            glm::vec3 keystoneMid = (peakF + peakB) * 0.5f;

            // Keystone block
            m = glm::scale(glm::translate(glm::mat4(1.f), keystoneMid),
                { BEAM_W * 1.80f, BEAM_H * 1.80f, ARCH_DEPTH + BEAM_W });
            drawCube(sh, m, glm::vec3(0.62f, 0.48f, 0.30f), texMarble, 1.f);

            // Keystone inner accent
            m = glm::scale(glm::translate(glm::mat4(1.f), keystoneMid),
                { BEAM_W * 1.10f, BEAM_H * 1.10f, ARCH_DEPTH + BEAM_W + 0.05f });
            drawCube(sh, m, glm::vec3(0.78f, 0.65f, 0.44f), texMarble, 1.f);
        }

        // ── Glowing campus name sign (centred beneath peak) ───────────────
        //   Hangs below the arch peak on the front face, slightly inset.
        //   In night mode it emits warm amber light.
        {
            glm::vec3 peakF   = bezierCubic(b0, b1, b2, b3, 0.5f);
            glm::vec3 signCtr = peakF + glm::vec3(0.f, -2.f, 0.f);
            float     signW   = (PILLAR_CX * 2.f) * 1.f;
            const glm::vec3 GOLD(0.92f, 0.78f, 0.28f);
            const glm::vec3 DARK(0.12f, 0.10f, 0.08f);

            // Sign backing board
            m = glm::scale(glm::translate(glm::mat4(1.f), signCtr),
                { signW, 0.62f, 0.12f });
            drawCube(sh, m, DARK, texWood, 1.f);

            // Gold lettering strip
            glm::vec3 em = dayMode ? ZERO3 : GOLD * 0.55f;
            glUniform3fv(glGetUniformLocation(sh,"emissive"),1,glm::value_ptr(em));
            m = glm::scale(glm::translate(glm::mat4(1.f), signCtr + glm::vec3(0.f,0.f,0.07f)),
                { signW * 0.92f, 0.40f, 0.08f });
            drawCube(sh, m, GOLD, texMarble, 1.f);

            // Three embossed letter-blocks spelling "KUET"
            float lW = 0.42f, lH = 0.30f, lD = 0.10f;
            float offsets[] = { -1.0f, -0.38f, 0.24f, 0.88f };
            for (int li = 0; li < 4; li++) {
                m = glm::scale(glm::translate(glm::mat4(1.f),
                    signCtr + glm::vec3(offsets[li], 0.f, -0.12f)),
                    { lW, lH, lD });
                drawCube(sh, m, DARK);
            }
            glUniform3fv(glGetUniformLocation(sh,"emissive"),1,glm::value_ptr(ZERO3));

            // Night-mode glow: thin emissive halo behind sign
            if (!dayMode) {
                glm::vec3 halo(0.85f, 0.68f, 0.22f);
                glUniform3fv(glGetUniformLocation(sh,"emissive"),1,glm::value_ptr(halo*0.40f));
                m = glm::scale(glm::translate(glm::mat4(1.f), signCtr + glm::vec3(0.f,0.f,0.06f)),
                    { signW + 0.20f, 0.80f, 0.06f });
                drawCube(sh, m, halo, texMarble, 1.f);
                glUniform3fv(glGetUniformLocation(sh,"emissive"),1,glm::value_ptr(ZERO3));
            }
        }

        // ── Pillar-to-arch transition moulding ────────────────────────────
        //   Small decorative collar where each arch leg meets the pillar capital.
        for (int s : {-1, 1}) {
            glm::vec3 archFoot = (s < 0) ? frontPts[0] : frontPts[N_SEG];
            m = glm::scale(glm::translate(glm::mat4(1.f),
                archFoot + glm::vec3(0.f, 0.f, -ARCH_DEPTH * 0.5f)),
                { BEAM_W + 0.30f, BEAM_H + 0.30f, ARCH_DEPTH + BEAM_W });
            drawCube(sh, m, CONC2, texConcrete, 1.f);
        }
    }

    // ── IRON GATE PANELS (unchanged) ─────────────────────────────────────
    float gateH = PHT * 0.82f;
    float gateW = GAP * 0.94f;
    for (int s : {-1, 1}) {
        float barX = GX + s * GAP * 0.50f;
        glm::mat4 door = glm::translate(glm::mat4(1.f), { barX, gateH * 0.50f, GZ });
        door = glm::rotate(door, glm::radians(s * 65.f), { 0.f, 1.f, 0.f });
        door = glm::scale(door, { gateW, gateH, 0.10f });
        drawCube(sh, door, IRON);

        // Vertical bars
        for (int b = -2; b <= 2; b++) {
            glm::mat4 bar = glm::translate(glm::mat4(1.f), { barX, gateH * 0.50f, GZ });
            bar = glm::rotate(bar, glm::radians(s * 65.f), { 0.f, 1.f, 0.f });
            bar = glm::translate(bar, { b * (gateW / 5.f), 0.f, 0.f });
            bar = glm::scale(bar, { 0.09f, gateH, 0.14f });
            drawCube(sh, bar, glm::vec3(0.26f, 0.26f, 0.28f));
        }

        // Horizontal rails
        for (int r : {-1, 1}) {
            glm::mat4 rail = glm::translate(glm::mat4(1.f), { barX, gateH * 0.50f, GZ });
            rail = glm::rotate(rail, glm::radians(s * 65.f), { 0.f, 1.f, 0.f });
            rail = glm::translate(rail, { 0.f, r * gateH * 0.46f, 0.f });
            rail = glm::scale(rail, { gateW, 0.10f, 0.14f });
            drawCube(sh, rail, IRON);
        }
    }
}
// ── Television stub (API compat) ─────────────────────────────────────────────
void renderTelevision(unsigned int sh) {}