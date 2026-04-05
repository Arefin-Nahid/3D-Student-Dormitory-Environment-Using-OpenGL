#include "scene_tennis.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

const float BC_X   = -52.f;
const float BC_Z   = -10.f;
const float BC_Y   =   0.06f;
const float BC_LEN =  23.77f;
const float BC_WID =  10.97f;
const float BC_SGL =   8.23f;

void renderTennisCourt(unsigned int sh) {
    glm::mat4 m;

    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, BC_Y - 0.03f, BC_Z }), { BC_WID + 3.f, 0.08f, BC_LEN + 3.f });
    drawCube(sh, m, glm::vec3(0.12f, 0.28f, 0.55f), texConcrete, 4.f);

    const float LT = 0.12f, LH = 0.04f, SY = BC_Y + 0.02f;
    const glm::vec3 WHITE(0.96f, 0.96f, 0.96f);

    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * BC_WID * 0.5f, SY, BC_Z }), { LT, LH, BC_LEN });
        drawCube(sh, m, WHITE);
    }
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * BC_SGL * 0.5f, SY, BC_Z }), { LT, LH, BC_LEN });
        drawCube(sh, m, WHITE);
    }
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, SY, BC_Z + s * BC_LEN * 0.5f }), { BC_WID, LH, LT });
        drawCube(sh, m, WHITE);
    }
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, SY, BC_Z + s * 6.4f }), { BC_SGL, LH, LT });
        drawCube(sh, m, WHITE);
    }
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, SY, BC_Z + s * 3.2f }), { LT, LH, 6.4f });
        drawCube(sh, m, WHITE);
    }
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, SY, BC_Z + s * (BC_LEN * 0.5f - 0.2f) }), { LT, LH, 0.4f });
        drawCube(sh, m, WHITE);
    }

    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * (BC_WID * 0.5f + 0.25f), 0.914f * 0.5f + BC_Y, BC_Z }), { 0.12f, 0.914f + BC_Y, 0.12f });
        drawCylinder(sh, m, glm::vec3(0.75f, 0.72f, 0.65f));
    }
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, BC_Y + 0.50f, BC_Z }), { BC_WID + 0.5f, 0.90f, 0.08f });
    drawCube(sh, m, glm::vec3(0.88f, 0.88f, 0.88f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, BC_Y + 0.96f, BC_Z }), { BC_WID + 0.5f, 0.08f, 0.12f });
    drawCube(sh, m, WHITE);
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, BC_Y + 0.50f, BC_Z }), { 0.08f, 0.90f, 0.10f });
    drawCube(sh, m, WHITE);

    const glm::vec3 FENCE(0.15f, 0.45f, 0.18f);
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * (BC_WID * 0.5f + 1.5f), 1.0f, BC_Z }), { 0.2f, 2.0f, BC_LEN + 5.f });
        drawCube(sh, m, FENCE, texGrass, 3.f);
    }
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, 1.0f, BC_Z + s * (BC_LEN * 0.5f + 1.5f) }), { BC_WID + 3.f, 2.0f, 0.2f });
        drawCube(sh, m, FENCE, texGrass, 3.f);
    }

    const glm::vec3 STEEL(0.50f, 0.50f, 0.52f);
    float poleOfsX = BC_WID * 0.5f + 2.5f, poleOfsZ = BC_LEN * 0.5f + 2.5f;
    for (int sx : {-1, 1}) for (int sz : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + sx * poleOfsX, 6.f, BC_Z + sz * poleOfsZ }), { 0.18f, 12.f, 0.18f });
        drawCylinder(sh, m, STEEL);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + sx * poleOfsX, 12.2f, BC_Z + sz * poleOfsZ }), { 0.5f, 0.28f, 0.8f });
        drawCube(sh, m, glm::vec3(0.90f, 0.88f, 0.22f));
        if (!dayMode) {
            glm::vec3 lemit(0.85f, 0.85f, 0.10f), zero(0.f, 0.f, 0.f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(lemit));
            m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + sx * poleOfsX, 11.6f, BC_Z + sz * poleOfsZ }), { 1.2f, 1.2f, 1.2f });
            drawCone(sh, m, glm::vec3(0.95f, 0.90f, 0.50f));
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(zero));
        }
    }

    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, 2.8f, BC_Z - BC_LEN * 0.5f - 1.6f }), { 4.0f, 0.8f, 0.15f });
    drawCube(sh, m, glm::vec3(0.12f, 0.12f, 0.45f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, 2.8f, BC_Z - BC_LEN * 0.5f - 1.55f }), { 3.6f, 0.55f, 0.08f });
    drawCube(sh, m, glm::vec3(0.95f, 0.95f, 0.92f));
}

void renderTennisPlayers(unsigned int sh) {
    float t = (float)glfwGetTime();
    float sway1 =  0.6f * sinf(t * 1.0f);
    float sway2 = -0.6f * sinf(t * 1.0f);

    const glm::vec3 SHIRT1(0.95f, 0.95f, 0.92f);
    const glm::vec3 SHIRT2(0.92f, 0.22f, 0.18f);
    const glm::vec3 SHORTS1(0.85f, 0.85f, 0.82f);
    const glm::vec3 SHORTS2(0.20f, 0.20f, 0.22f);
    const glm::vec3 SKIN(0.88f, 0.70f, 0.55f);
    const glm::vec3 SHOE(0.15f, 0.12f, 0.10f);
    const glm::vec3 RACKET1(0.25f, 0.22f, 0.22f);
    const glm::vec3 RACKET2(0.25f, 0.22f, 0.22f);

    auto drawPlayer = [&](float px, float pz, int dir,
        const glm::vec3& shirt, const glm::vec3& shorts, const glm::vec3& racket) {
        const float BY = 0.08f;
        glm::mat4 m;

        for (int s : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { px + s * 0.18f, BY + 0.12f, pz }), { 0.24f, 0.24f, 0.44f });
            drawCube(sh, m, SHOE);
        }
        for (int s : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { px + s * 0.18f, BY + 0.68f, pz }), { 0.22f, 0.82f, 0.22f });
            drawCube(sh, m, shorts);
        }
        m = glm::scale(glm::translate(glm::mat4(1.f), { px, BY + 1.04f, pz }), { 0.52f, 0.22f, 0.32f });
        drawCube(sh, m, shorts);
        m = glm::scale(glm::translate(glm::mat4(1.f), { px, BY + 1.60f, pz }), { 0.56f, 0.74f, 0.36f });
        drawCube(sh, m, shirt, texConcrete, 1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { px, BY + 2.22f, pz }), { 0.42f, 0.42f, 0.42f });
        drawSphere(sh, m, SKIN, texMarble, 1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { px, BY + 2.46f, pz + 0.08f }), { 0.46f, 0.16f, 0.48f });
        drawSphere(sh, m, glm::vec3(0.92f, 0.90f, 0.86f));

        float armAngle = 35.f + 20.f * sinf(t * 1.0f + (dir > 0 ? 0.f : PI));
        float armX = px + 0.40f, armBaseY = BY + 1.65f;
        glm::mat4 armBase = glm::translate(glm::mat4(1.f), { armX, armBaseY, pz });
        armBase = glm::rotate(armBase, glm::radians(armAngle), { 1.f, 0.f, 0.f });
        m = glm::scale(armBase, { 0.14f, 0.52f, 0.14f });
        drawCube(sh, m, SKIN);

        glm::mat4 forearmBase = armBase;
        forearmBase = glm::translate(forearmBase, { 0.f, 0.52f, 0.f });
        forearmBase = glm::rotate(forearmBase, glm::radians(armAngle * 0.5f), { 1.f,0.f,0.f });
        m = glm::scale(forearmBase, { 0.13f, 0.46f, 0.13f });
        drawCube(sh, m, SKIN);

        glm::mat4 handleBase = forearmBase;
        handleBase = glm::translate(handleBase, { 0.f, 0.46f, 0.f });
        m = glm::scale(handleBase, { 0.07f, 0.50f, 0.07f });
        drawCylinder(sh, m, glm::vec3(0.40f, 0.30f, 0.18f));

        glm::mat4 racketRoot = handleBase;
        racketRoot = glm::translate(racketRoot, { 0.f, 0.52f, 0.f });
        m = glm::scale(racketRoot, { 0.22f, 0.22f, 0.05f });
        drawCube(sh, m, racket);

        glm::mat4 racketHead = racketRoot;
        racketHead = glm::translate(racketHead, { 0.f, 0.30f, 0.f });
        m = glm::scale(racketHead, { 0.55f, 0.68f, 0.045f });
        drawCube(sh, m, racket);
        m = glm::scale(racketHead, { 0.44f, 0.58f, 0.050f });
        drawCube(sh, m, glm::vec3(0.90f, 0.90f, 0.85f));
    };

    drawPlayer(BC_X + sway1, BC_Z - 5.0f, +1, SHIRT1, SHORTS1, RACKET1);
    drawPlayer(BC_X + sway2, BC_Z + 5.0f, -1, SHIRT2, SHORTS2, RACKET2);
}

void renderTennisBall(unsigned int sh) {
    float t = (float)glfwGetTime();
    const float PERIOD = 2.4f;
    float phase = fmodf(t, PERIOD) / PERIOD;

    float halfPhase, dirSign;
    if (phase < 0.5f) { halfPhase = phase * 2.f; dirSign = 1.f; }
    else              { halfPhase = (phase - 0.5f) * 2.f; dirSign = -1.f; }

    const float ZS_P = BC_Z - 4.5f, ZN_P = BC_Z + 4.5f;
    float ballZ = ZS_P + (ZN_P - ZS_P) * halfPhase;
    const float BASE_Y = 1.2f, PEAK_Y = 5.0f;
    float yNorm = 4.f * halfPhase * (1.f - halfPhase);
    float ballY = BASE_Y + (PEAK_Y - BASE_Y) * yNorm;
    float ballX = BC_X + 0.5f * sinf(t * 1.0f);
    float spinAngle = t * 540.f;

    glm::mat4 base = glm::translate(glm::mat4(1.f), { ballX, ballY, ballZ });
    base = glm::rotate(base, glm::radians(spinAngle), { 1.f, 0.f, 0.f });

    glm::mat4 m = glm::scale(base, { 0.18f, 0.18f, 0.18f });
    drawSphere(sh, m, glm::vec3(0.75f, 0.85f, 0.12f), texGrass, 1.f);

    glm::mat4 seamMat = glm::rotate(base, glm::radians(45.f), { 0.f, 0.f, 1.f });
    seamMat = glm::scale(seamMat, { 0.165f, 0.02f, 0.165f });
    drawCylinder(sh, seamMat, glm::vec3(0.95f, 0.95f, 0.92f));

    float shadowScale = 0.20f + 0.12f * (1.f - yNorm);
    glm::mat4 shadow = glm::scale(glm::translate(glm::mat4(1.f), { ballX, BC_Y + 0.01f, ballZ }), { shadowScale * 2.5f, 0.02f, shadowScale * 2.5f });
    drawCylinder(sh, shadow, glm::vec3(0.04f, 0.08f, 0.18f));
}
