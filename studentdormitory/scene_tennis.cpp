#include "scene_tennis.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// Tennis court: LEFT side of campus
const float BC_X = -52.f;
const float BC_Z = -10.f;
const float BC_Y = 0.06f;
const float BC_LEN = 23.77f;
const float BC_WID = 10.97f;
const float BC_SGL = 8.23f;

void renderTennisCourt(unsigned int sh) {
    glm::mat4 m;
    // Court surface (hard blue)
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X,BC_Y - 0.03f,BC_Z }), { BC_WID + 3.f,0.08f,BC_LEN + 3.f });
    drawCube(sh, m, glm::vec3(0.12f, 0.28f, 0.55f), texConcrete, 4.f);

    const float LT = 0.12f, LH = 0.04f, SY = BC_Y + 0.02f;
    const glm::vec3 W(0.96f, 0.96f, 0.96f);

    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * BC_WID * 0.5f,SY,BC_Z }), { LT,LH,BC_LEN });
        drawCube(sh, m, W);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * BC_SGL * 0.5f,SY,BC_Z }), { LT,LH,BC_LEN });
        drawCube(sh, m, W);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X,SY,BC_Z + s * BC_LEN * 0.5f }), { BC_WID,LH,LT });
        drawCube(sh, m, W);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X,SY,BC_Z + s * 6.4f }), { BC_SGL,LH,LT });
        drawCube(sh, m, W);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X,SY,BC_Z + s * 3.2f }), { LT,LH,6.4f });
        drawCube(sh, m, W);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X,SY,BC_Z + s * (BC_LEN * 0.5f - 0.2f) }), { LT,LH,0.4f });
        drawCube(sh, m, W);
    }

    // Net posts
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * (BC_WID * 0.5f + 0.25f),0.914f * 0.5f + BC_Y,BC_Z }), { 0.12f,0.914f + BC_Y,0.12f });
        drawCylinder(sh, m, glm::vec3(0.75f, 0.72f, 0.65f));
    }
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X,BC_Y + 0.50f,BC_Z }), { BC_WID + 0.5f,0.90f,0.08f });
    drawCube(sh, m, glm::vec3(0.88f, 0.88f, 0.88f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X,BC_Y + 0.96f,BC_Z }), { BC_WID + 0.5f,0.08f,0.12f });
    drawCube(sh, m, W);
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X,BC_Y + 0.50f,BC_Z }), { 0.08f,0.90f,0.10f });
    drawCube(sh, m, W);

    // Windbreak fences
    const glm::vec3 FENCE(0.15f, 0.45f, 0.18f);
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * (BC_WID * 0.5f + 1.5f),1.0f,BC_Z }), { 0.2f,2.0f,BC_LEN + 5.f });
        drawCube(sh, m, FENCE, texGrass, 3.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X,1.0f,BC_Z + s * (BC_LEN * 0.5f + 1.5f) }), { BC_WID + 3.f,2.0f,0.2f });
        drawCube(sh, m, FENCE, texGrass, 3.f);
    }

    // Floodlight poles
    const glm::vec3 STEEL(0.50f, 0.50f, 0.52f);
    float pOX = BC_WID * 0.5f + 2.5f, pOZ = BC_LEN * 0.5f + 2.5f;
    for (int sx : {-1, 1}) for (int sz : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + sx * pOX,6.f,BC_Z + sz * pOZ }), { 0.18f,12.f,0.18f });
        drawCylinder(sh, m, STEEL);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + sx * pOX,12.2f,BC_Z + sz * pOZ }), { 0.5f,0.28f,0.8f });
        drawCube(sh, m, glm::vec3(0.90f, 0.88f, 0.22f));
        if (!dayMode) {
            glm::vec3 lemit(0.85f, 0.85f, 0.10f), zero(0.f, 0.f, 0.f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(lemit));
            m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + sx * pOX,11.6f,BC_Z + sz * pOZ }), { 1.2f,1.2f,1.2f });
            drawCone(sh, m, glm::vec3(0.95f, 0.90f, 0.50f));
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(zero));
        }
    }

    // Court name sign
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X,2.8f,BC_Z - BC_LEN * 0.5f - 1.6f }), { 4.0f,0.8f,0.15f });
    drawCube(sh, m, glm::vec3(0.12f, 0.12f, 0.45f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X,2.8f,BC_Z - BC_LEN * 0.5f - 1.55f }), { 3.6f,0.55f,0.08f });
    drawCube(sh, m, glm::vec3(0.95f, 0.95f, 0.92f));
}

void renderTennisPlayers(unsigned int sh) {
    float t = (float)glfwGetTime();
    float sway1 = 0.6f * sinf(t * 1.0f);
    float sway2 = -0.6f * sinf(t * 1.0f);

    const glm::vec3 SHIRT1(0.95f, 0.95f, 0.92f), SHIRT2(0.92f, 0.22f, 0.18f);
    const glm::vec3 SHORTS1(0.85f, 0.85f, 0.82f), SHORTS2(0.20f, 0.20f, 0.22f);
    const glm::vec3 SKIN(0.88f, 0.70f, 0.55f), SHOE(0.15f, 0.12f, 0.10f);
    const glm::vec3 RACKET(0.25f, 0.22f, 0.22f);
    const float BY = 0.08f;

    auto drawPlayer = [&](float px, float pz, int dir,
        const glm::vec3& shirt, const glm::vec3& shorts, const glm::vec3& racket) {
            glm::mat4 m;
            for (int s : {-1, 1}) { m = glm::scale(glm::translate(glm::mat4(1.f), { px + s * 0.18f,BY + 0.12f,pz }), { 0.24f,0.24f,0.44f }); drawCube(sh, m, SHOE); }
            for (int s : {-1, 1}) { m = glm::scale(glm::translate(glm::mat4(1.f), { px + s * 0.18f,BY + 0.68f,pz }), { 0.22f,0.82f,0.22f }); drawCube(sh, m, shorts); }
            m = glm::scale(glm::translate(glm::mat4(1.f), { px,BY + 1.04f,pz }), { 0.52f,0.22f,0.32f }); drawCube(sh, m, shorts);
            m = glm::scale(glm::translate(glm::mat4(1.f), { px,BY + 1.60f,pz }), { 0.56f,0.74f,0.36f }); drawCube(sh, m, shirt, texConcrete, 1.f);
            m = glm::scale(glm::translate(glm::mat4(1.f), { px,BY + 2.22f,pz }), { 0.42f,0.42f,0.42f }); drawSphere(sh, m, SKIN, texMarble, 1.f);
            float armAngle = 35.f + 20.f * sinf(t * 1.0f + (dir > 0 ? 0.f : PI));
            glm::mat4 armBase = glm::translate(glm::mat4(1.f), { px + 0.40f,BY + 1.65f,pz });
            armBase = glm::rotate(armBase, glm::radians(armAngle), { 1.f,0.f,0.f });
            m = glm::scale(armBase, { 0.14f,0.52f,0.14f }); drawCube(sh, m, SKIN);
            glm::mat4 forearmBase = glm::translate(armBase, { 0.f,0.52f,0.f });
            forearmBase = glm::rotate(forearmBase, glm::radians(armAngle * 0.5f), { 1.f,0.f,0.f });
            m = glm::scale(forearmBase, { 0.13f,0.46f,0.13f }); drawCube(sh, m, SKIN);
            glm::mat4 handleBase = glm::translate(forearmBase, { 0.f,0.46f,0.f });
            m = glm::scale(handleBase, { 0.07f,0.50f,0.07f }); drawCylinder(sh, m, glm::vec3(0.40f, 0.30f, 0.18f));
            glm::mat4 racketRoot = glm::translate(handleBase, { 0.f,0.52f,0.f });
            m = glm::scale(racketRoot, { 0.22f,0.22f,0.05f }); drawCube(sh, m, racket);
            glm::mat4 racketHead = glm::translate(racketRoot, { 0.f,0.30f,0.f });
            m = glm::scale(racketHead, { 0.55f,0.68f,0.045f }); drawCube(sh, m, racket);
            m = glm::scale(racketHead, { 0.44f,0.58f,0.050f }); drawCube(sh, m, glm::vec3(0.90f, 0.90f, 0.85f));
        };
    drawPlayer(BC_X + sway1, BC_Z - 5.0f, +1, SHIRT1, SHORTS1, RACKET);
    drawPlayer(BC_X + sway2, BC_Z + 5.0f, -1, SHIRT2, SHORTS2, RACKET);
}

// ── Tennis ball with correct physics: parabolic arc + bounce on court surface ──
void renderTennisBall(unsigned int sh) {
    float t = (float)glfwGetTime();

    // Ball travels south-to-north (Z axis), bounces on court surface
    // Physics: simulate projectile with gravity-like parabola, bounce when y <= court surface
    const float SPEED = 8.0f;        // units/second across court half
    const float COURT_Y = BC_Y + 0.09f; // court surface + ball radius
    const float LAUNCH_V = 5.5f;        // initial vertical velocity at launch
    const float GRAVITY = 14.0f;       // downward acceleration
    const float HALF_LEN = BC_LEN * 0.5f - 0.5f;

    // Full round-trip time
    float tripTime = HALF_LEN / SPEED;  // one-way flight time along Z
    float cyclePeriod = tripTime * 2.0f; // round trip

    float ct = fmodf(t, cyclePeriod);
    float phase, dirZ;
    if (ct < tripTime) {
        phase = ct / tripTime;       // 0->1 going +Z
        dirZ = 1.f;
    }
    else {
        phase = (ct - tripTime) / tripTime; // 0->1 going -Z
        dirZ = -1.f;
    }

    // Z position: linear across court half
    float ballZ = BC_Z + dirZ * (phase - 0.5f) * (2.f * HALF_LEN);

    // Y position: segment into 2 bounces per half-court
    // Use sinf to model a clean parabola that always lands on the surface
    float bounce = PI * phase * 2.f;       // two arcs per half trip
    float rawY = sinf(bounce);           // 0..1..0..1..0 (always >= 0)
    float ballY = COURT_Y + fabsf(rawY) * 3.2f; // peak height ~3.2 above court

    // Slight lateral drift
    float ballX = BC_X + 0.4f * sinf(t * 1.5f);

    // Spin
    float spinAngle = t * 540.f;
    glm::mat4 base = glm::translate(glm::mat4(1.f), { ballX,ballY,ballZ });
    base = glm::rotate(base, glm::radians(spinAngle), { 1.f,0.f,0.f });

    // Draw ball
    glm::mat4 m = glm::scale(base, { 0.18f,0.18f,0.18f });
    drawSphere(sh, m, glm::vec3(0.75f, 0.85f, 0.12f), texGrass, 1.f);

    // Seam stripe
    glm::mat4 seam = glm::rotate(base, glm::radians(45.f), { 0.f,0.f,1.f });
    seam = glm::scale(seam, { 0.165f,0.02f,0.165f });
    drawCylinder(sh, seam, glm::vec3(0.95f, 0.95f, 0.92f));

    // Shadow on court (scales down as ball rises)
    float normY = (ballY - COURT_Y) / 3.2f;  // 0 at surface, 1 at peak
    float shadowSc = 0.20f + 0.14f * (1.f - normY);
    m = glm::scale(glm::translate(glm::mat4(1.f), { ballX,BC_Y + 0.01f,ballZ }),
        { shadowSc * 2.5f,0.02f,shadowSc * 2.5f });
    drawCylinder(sh, m, glm::vec3(0.04f, 0.08f, 0.18f));
}