#include "scene_football.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>

const float CR_X     =  52.f;
const float CR_Z     = -10.f;
const float CR_Y     =   0.06f;
const float CR_FLEN  =  40.f;
const float CR_FWID  =  25.f;
const float CR_CYCLE =   3.0f;

void renderFootballField(unsigned int sh) {
    glm::mat4 m;

    m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X, CR_Y - 0.04f, CR_Z }), { CR_FWID + 4.f, 0.06f, CR_FLEN + 4.f });
    drawCube(sh, m, glm::vec3(0.20f, 0.48f, 0.15f), texGrass, 8.f);

    for (int i = 0; i < 5; i++) {
        float sz = CR_Z - CR_FLEN * 0.5f + i * (CR_FLEN / 5.f) + CR_FLEN / 10.f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X, CR_Y - 0.02f, sz }), { CR_FWID, 0.065f, CR_FLEN / 5.f * 0.92f });
        drawCube(sh, m, glm::vec3(0.16f, 0.40f, 0.12f), texGrass, 4.f);
    }

    const glm::vec3 WHITE(0.96f, 0.96f, 0.96f);
    const float LH = 0.04f, LT = 0.14f, SY = CR_Y + 0.04f;

    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X + s * CR_FWID * 0.5f, SY, CR_Z }), { LT, LH, CR_FLEN });
        drawCube(sh, m, WHITE);
    }
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X, SY, CR_Z + s * CR_FLEN * 0.5f }), { CR_FWID, LH, LT });
        drawCube(sh, m, WHITE);
    }
    m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X, SY, CR_Z }), { CR_FWID, LH, LT });
    drawCube(sh, m, WHITE);
    for (int i = 0; i < 24; i++) {
        float ang = 2.f * PI * i / 24.f;
        float cx = CR_X + 4.5f * cosf(ang), cz = CR_Z + 4.5f * sinf(ang);
        m = glm::scale(glm::translate(glm::mat4(1.f), { cx, SY, cz }), { LT, LH, LT });
        drawCube(sh, m, WHITE);
    }
    m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X, SY, CR_Z }), { 0.28f, LH, 0.28f });
    drawCube(sh, m, WHITE);

    float paW = 10.f, paD = 5.5f;
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X, SY, CR_Z + s * (CR_FLEN * 0.5f - paD) }), { paW, LH, LT });
        drawCube(sh, m, WHITE);
        for (int sx : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X + sx * paW * 0.5f, SY, CR_Z + s * (CR_FLEN * 0.5f - paD * 0.5f) }), { LT, LH, paD });
            drawCube(sh, m, WHITE);
        }
        m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X, SY, CR_Z + s * (CR_FLEN * 0.5f - paD - 1.5f) }), { 0.24f, LH, 0.24f });
        drawCube(sh, m, WHITE);
    }

    const glm::vec3 POST(0.95f, 0.95f, 0.92f);
    float goalW = 5.0f, goalH = 2.5f, goalD = 1.2f;
    for (int s : {-1, 1}) {
        float gz = CR_Z + s * CR_FLEN * 0.5f;
        for (int gx : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X + gx * goalW * 0.5f, goalH * 0.5f + CR_Y, gz }), { 0.15f, goalH, 0.15f });
            drawCylinder(sh, m, POST);
        }
        m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X, goalH + CR_Y, gz }), { goalW, 0.15f, 0.15f });
        drawCube(sh, m, POST);
        for (int gx : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X + gx * goalW * 0.5f, goalH * 0.5f, gz + s * goalD }), { 0.12f, goalH, 0.12f });
            drawCylinder(sh, m, POST);
        }
        m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X, goalH * 0.5f, gz + s * goalD }), { goalW, goalH, 0.10f });
        drawCube(sh, m, glm::vec3(0.92f, 0.92f, 0.88f));
        for (int gx : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X + gx * goalW * 0.5f, goalH * 0.5f, gz + s * goalD * 0.5f }), { 0.10f, goalH, goalD });
            drawCube(sh, m, glm::vec3(0.92f, 0.92f, 0.88f));
        }
    }

    const glm::vec3 FLAG_RED(0.85f, 0.15f, 0.15f);
    float cfOX = CR_FWID * 0.5f, cfOZ = CR_FLEN * 0.5f;
    for (int sx : {-1, 1}) for (int sz : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X + sx * cfOX, 1.5f, CR_Z + sz * cfOZ }), { 0.06f, 3.0f, 0.06f });
        drawCylinder(sh, m, glm::vec3(0.85f, 0.82f, 0.75f));
        m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X + sx * cfOX + 0.2f, 3.1f, CR_Z + sz * cfOZ }), { 0.45f, 0.3f, 0.08f });
        drawCube(sh, m, FLAG_RED);
    }

    const glm::vec3 STAND_COL(0.72f, 0.70f, 0.68f);
    for (int row = 0; row < 5; row++) {
        float rowZ = CR_Z - CR_FLEN * 0.5f - 2.0f - row * 1.2f;
        float rowH = 0.6f + row * 0.6f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X, rowH * 0.5f, rowZ }), { CR_FWID * 1.3f, rowH, 1.0f });
        drawCube(sh, m, STAND_COL, texConcrete, 4.f);
        glm::vec3 specCols[] = { {0.85f,0.12f,0.12f}, {0.18f,0.38f,0.85f}, {0.18f,0.68f,0.22f}, {0.88f,0.78f,0.12f}, {0.85f,0.42f,0.12f}, {0.68f,0.18f,0.68f} };
        int nSpec = (int)(CR_FWID * 1.1f);
        for (int sp = 0; sp < nSpec; sp++) {
            float spX = CR_X - CR_FWID * 0.65f + sp * (CR_FWID * 1.3f / nSpec);
            m = glm::scale(glm::translate(glm::mat4(1.f), { spX, rowH + 0.25f, rowZ }), { 0.42f, 0.45f, 0.38f });
            drawCube(sh, m, specCols[sp % 6]);
        }
    }
    for (int row = 0; row < 5; row++) {
        float rowZ = CR_Z + CR_FLEN * 0.5f + 2.0f + row * 1.2f;
        float rowH = 0.6f + row * 0.6f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X, rowH * 0.5f, rowZ }), { CR_FWID * 1.3f, rowH, 1.0f });
        drawCube(sh, m, STAND_COL, texConcrete, 4.f);
        glm::vec3 specCols[] = { {0.85f,0.12f,0.12f}, {0.18f,0.38f,0.85f}, {0.18f,0.68f,0.22f}, {0.88f,0.78f,0.12f}, {0.85f,0.42f,0.12f}, {0.68f,0.18f,0.68f} };
        int nSpec = (int)(CR_FWID * 1.1f);
        for (int sp = 0; sp < nSpec; sp++) {
            float spX = CR_X - CR_FWID * 0.65f + sp * (CR_FWID * 1.3f / nSpec);
            m = glm::scale(glm::translate(glm::mat4(1.f), { spX, rowH + 0.25f, rowZ }), { 0.42f, 0.45f, 0.38f });
            drawCube(sh, m, specCols[sp % 6]);
        }
    }
    for (int row = 0; row < 4; row++) {
        float rowX = CR_X + CR_FWID * 0.5f + 2.0f + row * 1.2f;
        float rowH = 0.5f + row * 0.55f;
        m = glm::scale(glm::translate(glm::mat4(1.f), { rowX, rowH * 0.5f, CR_Z }), { 1.0f, rowH, CR_FLEN * 1.1f });
        drawCube(sh, m, STAND_COL, texConcrete, 5.f);
        glm::vec3 specCols[] = { {0.85f,0.12f,0.12f}, {0.18f,0.38f,0.85f}, {0.18f,0.68f,0.22f}, {0.88f,0.78f,0.12f} };
        int nSpec = (int)(CR_FLEN * 0.85f);
        for (int sp = 0; sp < nSpec; sp++) {
            float spZ = CR_Z - CR_FLEN * 0.55f + sp * (CR_FLEN * 1.1f / nSpec);
            m = glm::scale(glm::translate(glm::mat4(1.f), { rowX, rowH + 0.22f, spZ }), { 0.38f, 0.42f, 0.38f });
            drawCube(sh, m, specCols[sp % 4]);
        }
    }

    float sbX = CR_X + CR_FWID * 0.5f + 7.5f, sbZ = CR_Z - 5.f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { sbX, 4.0f, sbZ }), { 0.3f, 5.5f, 8.0f });
    drawCube(sh, m, glm::vec3(0.12f, 0.12f, 0.18f), texConcrete, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { sbX + 0.18f, 4.2f, sbZ }), { 0.08f, 4.0f, 7.0f });
    drawCube(sh, m, glm::vec3(0.10f, 0.75f, 0.22f));
    for (int sleg : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { sbX, 2.f, sbZ + sleg * 3.0f }), { 0.28f, 4.0f, 0.28f });
        drawCylinder(sh, m, glm::vec3(0.35f, 0.33f, 0.30f));
    }

    const glm::vec3 STEEL(0.50f, 0.50f, 0.52f);
    for (int sx : {-1, 1}) for (int sz : {-1, 1}) {
        float tx = CR_X + sx * (CR_FWID * 0.5f + 3.f);
        float tz = CR_Z + sz * (CR_FLEN * 0.5f + 3.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { tx, 10.f, tz }), { 0.22f, 20.f, 0.22f });
        drawCylinder(sh, m, STEEL);
        for (int li = -1; li <= 1; li++) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { tx + li * 0.6f, 20.3f, tz }), { 0.55f, 0.32f, 0.9f });
            drawCube(sh, m, glm::vec3(0.90f, 0.88f, 0.22f));
        }
        if (!dayMode) {
            glm::vec3 lemit(0.85f, 0.85f, 0.10f), zero(0.f, 0.f, 0.f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(lemit));
            m = glm::scale(glm::translate(glm::mat4(1.f), { tx, 19.5f, tz }), { 2.5f, 2.5f, 2.5f });
            drawCone(sh, m, glm::vec3(0.95f, 0.90f, 0.50f));
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(zero));
        }
    }
}

void renderFootballPlayers(unsigned int sh) {
    float t = (float)glfwGetTime();
    float phase = fmodf(t, CR_CYCLE) / CR_CYCLE;

    const glm::vec3 SKIN(0.88f, 0.70f, 0.55f);
    const glm::vec3 SHOE(0.12f, 0.10f, 0.10f);
    const glm::vec3 SHIRT_A(0.18f, 0.32f, 0.82f);
    const glm::vec3 SHIRT_B(0.85f, 0.20f, 0.20f);
    const glm::vec3 SHORTS_A(0.14f, 0.14f, 0.60f);
    const glm::vec3 SHORTS_B(0.65f, 0.12f, 0.12f);
    const glm::vec3 GK_SHIRT(0.22f, 0.68f, 0.28f);
    float bY = CR_Y;

    auto drawFootballer = [&](float px, float pz, const glm::vec3& shirt, const glm::vec3& shrt, float legAngle1, float legAngle2) {
        glm::mat4 m;
        for (int s : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { px + s * 0.16f, bY + 0.12f, pz }), { 0.22f, 0.22f, 0.40f });
            drawCube(sh, m, SHOE);
        }
        {
            glm::mat4 leg = glm::translate(glm::mat4(1.f), { px - 0.15f, bY + 0.68f, pz });
            leg = glm::rotate(leg, glm::radians(legAngle1), { 1.f,0.f,0.f });
            m = glm::scale(leg, { 0.22f, 0.82f, 0.22f });
            drawCube(sh, m, shrt);
        }
        {
            glm::mat4 leg = glm::translate(glm::mat4(1.f), { px + 0.15f, bY + 0.68f, pz });
            leg = glm::rotate(leg, glm::radians(legAngle2), { 1.f,0.f,0.f });
            m = glm::scale(leg, { 0.22f, 0.82f, 0.22f });
            drawCube(sh, m, shrt);
        }
        m = glm::scale(glm::translate(glm::mat4(1.f), { px, bY + 1.60f, pz }), { 0.54f, 0.72f, 0.34f });
        drawCube(sh, m, shirt, texConcrete, 1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { px, bY + 2.22f, pz }), { 0.42f, 0.42f, 0.42f });
        drawSphere(sh, m, SKIN, texMarble, 1.f);
        for (int s : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { px + s * 0.38f, bY + 1.65f, pz }), { 0.14f, 0.55f, 0.14f });
            drawCube(sh, m, shirt);
        }
    };

    float strikerZ = CR_Z - CR_FLEN * 0.3f + phase * CR_FLEN * 0.6f;
    float strideA  = 35.f * sinf(t * 4.f);
    drawFootballer(CR_X - 2.f, strikerZ, SHIRT_A, SHORTS_A, strideA, -strideA);

    float gkSway = 1.8f * sinf(t * 1.2f);
    drawFootballer(CR_X + gkSway, CR_Z - CR_FLEN * 0.5f + 0.8f, GK_SHIRT, glm::vec3(0.18f, 0.50f, 0.20f), 0.f, 0.f);

    struct PPos { float x, z; glm::vec3 sh; glm::vec3 sh2; };
    PPos pp[] = {
        { CR_X - 8.f, CR_Z - 8.f, SHIRT_A, SHORTS_A },
        { CR_X + 8.f, CR_Z - 8.f, SHIRT_A, SHORTS_A },
        { CR_X - 6.f, CR_Z + 5.f, SHIRT_B, SHORTS_B },
        { CR_X + 6.f, CR_Z + 5.f, SHIRT_B, SHORTS_B },
    };
    for (auto& p : pp) {
        float ls = 12.f * sinf(t * 1.5f + p.x);
        drawFootballer(p.x, p.z, p.sh, p.sh2, ls, -ls);
    }
}

void renderFootball(unsigned int sh) {
    float t = (float)glfwGetTime();
    float phase = fmodf(t, CR_CYCLE) / CR_CYCLE;

    float strikerZ = CR_Z - CR_FLEN * 0.3f + phase * CR_FLEN * 0.6f;
    float ballX = CR_X - 2.f + 0.3f * sinf(t * 4.2f);
    float ballZ, ballY;

    if (phase < 0.75f) {
        ballZ = strikerZ + 0.5f;
        ballY = CR_Y + 0.18f + 0.05f * fabsf(sinf(t * 8.f));
    }
    else {
        float p = (phase - 0.75f) / 0.25f;
        float goalZ = CR_Z + CR_FLEN * 0.5f;
        ballZ = strikerZ + p * (goalZ - strikerZ);
        ballX = CR_X - 2.f + p * 2.f;
        float arc = 4.f * p * (1.f - p);
        ballY = CR_Y + 0.18f + arc * 3.5f;
    }

    float spinAngle = t * 480.f;
    glm::mat4 base = glm::translate(glm::mat4(1.f), { ballX, ballY, ballZ });
    base = glm::rotate(base, glm::radians(spinAngle), { 1.f, 0.f, 0.f });

    glm::mat4 m = glm::scale(base, { 0.22f, 0.22f, 0.22f });
    drawSphere(sh, m, glm::vec3(0.96f, 0.96f, 0.94f), texMarble, 1.f);

    const glm::vec3 BLACK(0.08f, 0.08f, 0.08f);
    for (int i = 0; i < 6; i++) {
        float ang = i * 60.f;
        glm::mat4 p2 = glm::rotate(base, glm::radians(ang), { 0.f,1.f,0.f });
        p2 = glm::translate(p2, { 0.f, 0.22f, 0.f });
        m = glm::scale(p2, { 0.12f, 0.03f, 0.12f });
        drawCylinder(sh, m, BLACK);
    }

    float shadowSc = 0.22f + 0.08f * (1.f - std::min((ballY - CR_Y) / 4.0f, 1.f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { ballX, CR_Y + 0.005f, ballZ }), { shadowSc * 2.5f, 0.015f, shadowSc * 2.5f });
    drawCylinder(sh, m, glm::vec3(0.04f, 0.09f, 0.04f));
}
