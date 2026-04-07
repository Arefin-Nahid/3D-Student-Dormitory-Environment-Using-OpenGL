#include "scene_football.h"
#include "match_state.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>

const float CR_X = 52.f;
const float CR_Z = 0.f;
const float CR_Y = 0.06f;
const float CR_FLEN = 40.f;
const float CR_FWID = 25.f;
const float CR_CYCLE = 3.0f;

// ─── Player initialisation ────────────────────────────────────────────────────
void FootballMatch::initPlayers() {
    const glm::vec3 SHIRT_A(0.18f, 0.32f, 0.82f), SHORTS_A(0.14f, 0.14f, 0.60f);
    const glm::vec3 SHIRT_B(0.85f, 0.20f, 0.20f), SHORTS_B(0.65f, 0.12f, 0.12f);
    const glm::vec3 GK(0.22f, 0.68f, 0.28f), GK_S(0.18f, 0.50f, 0.20f);

    // Team A (south): players[0]=striker, players[1]=mid, players[2]=GK
    players[0] = { { FIELD_X - 2.f, 0.f, -8.f }, {}, SHIRT_A, SHORTS_A, false, 0.f };
    players[1] = { { FIELD_X + 4.f, 0.f,  0.f }, {}, SHIRT_A, SHORTS_A, false, 0.f };
    players[2] = { { FIELD_X,       0.f,  FIELD_Z - 1.f }, {}, GK, GK_S, true, 0.f };

    // Team B (north): players[3]=striker, players[4]=mid, players[5]=GK
    players[3] = { { FIELD_X + 2.f, 0.f,  8.f }, {}, SHIRT_B, SHORTS_B, false, 0.f };
    players[4] = { { FIELD_X - 4.f, 0.f,  0.f }, {}, SHIRT_B, SHORTS_B, false, 0.f };
    players[5] = { { FIELD_X,       0.f, -FIELD_Z + 1.f }, {}, GK, GK_S, true, 0.f };
}

// ─── Physics update ───────────────────────────────────────────────────────────
static void updateFootballPhysics(float dt) {
    FootballMatch& M = gFootballMatch;
    if (!M.active || M.paused) return;

    M.matchTime += dt;

    // ── Goal-celebration / reset timer ───────────────────────────────────────
    if (M.goalCelebration) {
        M.resetTimer -= dt;
        if (M.resetTimer <= 0.f) {
            M.goalCelebration = false;
            M.resetBall();
            M.initPlayers();
        }
        return;  // freeze physics during celebration
    }

    FootballBall& B = M.ball;

    // ── Ball gravity & integration ────────────────────────────────────────────
    B.vel.y -= FootballBall::GRAVITY * dt;
    B.pos += B.vel * dt;
    B.spinAngle += glm::length(glm::vec2(B.vel.x, B.vel.z)) * 180.f * dt;

    // ── Ground collision ──────────────────────────────────────────────────────
    float groundY = FootballBall::GROUND_Y + B.radius;
    if (B.pos.y < groundY) {
        B.pos.y = groundY;
        if (B.vel.y < 0.f) {
            B.vel.y *= -FootballBall::BOUNCE_COR;
            if (fabsf(B.vel.y) < 0.25f) B.vel.y = 0.f;  // stop tiny bounces
        }
        // Friction on ground contact
        B.vel.x *= FootballBall::FRICTION;
        B.vel.z *= FootballBall::FRICTION;
    }

    // ── Field boundary (side walls) ───────────────────────────────────────────
    float hwid = CR_FWID * 0.5f;
    if (B.pos.x < CR_X - hwid + B.radius) { B.pos.x = CR_X - hwid + B.radius; B.vel.x = fabsf(B.vel.x) * 0.5f; }
    if (B.pos.x > CR_X + hwid - B.radius) { B.pos.x = CR_X + hwid - B.radius; B.vel.x = -fabsf(B.vel.x) * 0.5f; }

    // ── Goal detection ────────────────────────────────────────────────────────
    float halfGoal = FootballMatch::GOAL_W * 0.5f;
    // North goal (positive Z end) → Team A scores
    if (B.pos.z > CR_Z + CR_FLEN * 0.5f &&
        fabsf(B.pos.x - CR_X) < halfGoal &&
        B.pos.y < FootballMatch::GOAL_H) {
        M.scoreA++;
        std::cout << "⚽ GOAL! Team A scores!  Score: " << M.scoreA << " - " << M.scoreB << std::endl;
        M.goalCelebration = true; M.resetTimer = 3.0f;
        return;
    }
    // South goal (negative Z end) → Team B scores
    if (B.pos.z < CR_Z - CR_FLEN * 0.5f &&
        fabsf(B.pos.x - CR_X) < halfGoal &&
        B.pos.y < FootballMatch::GOAL_H) {
        M.scoreB++;
        std::cout << "⚽ GOAL! Team B scores!  Score: " << M.scoreA << " - " << M.scoreB << std::endl;
        M.goalCelebration = true; M.resetTimer = 3.0f;
        return;
    }
    // Ball out of bounds → reset without goal
    if (fabsf(B.pos.z - CR_Z) > CR_FLEN * 0.5f + 1.5f) {
        M.resetBall();
    }

    // ── Player AI & kick ─────────────────────────────────────────────────────
    for (int i = 0; i < FootballMatch::NUM_PLAYERS; i++) {
        FootballPlayer& P = M.players[i];
        P.kickCooldown -= dt;

        glm::vec3 toBall = B.pos - P.pos;
        toBall.y = 0.f;
        float dist = glm::length(toBall);

        // GK stays near goal line; outfield players chase ball
        glm::vec3 targetPos;
        if (P.isGoalkeeper) {
            // Lateral tracking only – don't leave goal mouth
            float goalZ = (i == 2) ? (CR_Z + CR_FLEN * 0.5f - 1.f) : (CR_Z - CR_FLEN * 0.5f + 1.f);
            targetPos = { CR_X + (B.pos.x - CR_X) * 0.6f, 0.f, goalZ };
        }
        else {
            targetPos = B.pos; targetPos.y = 0.f;
        }

        glm::vec3 dir = targetPos - P.pos;
        dir.y = 0.f;
        float d2 = glm::length(dir);
        if (d2 > 0.05f) {
            dir /= d2;
            P.pos += dir * FootballPlayer::SPEED * dt;
        }

        // Clamp player to field
        P.pos.x = glm::clamp(P.pos.x, CR_X - hwid + 0.3f, CR_X + hwid - 0.3f);
        P.pos.z = glm::clamp(P.pos.z, CR_Z - CR_FLEN * 0.5f + 0.3f, CR_Z + CR_FLEN * 0.5f - 0.3f);

        // Kick
        if (dist < FootballPlayer::KICK_RANGE && P.kickCooldown <= 0.f) {
            P.kickCooldown = FootballPlayer::KICK_CD;

            // Determine kick target: players[0-2] kick north, [3-5] kick south
            bool kickNorth = (i < 3);
            glm::vec3 targetGoal = { CR_X, 0.f, kickNorth ? CR_Z + CR_FLEN * 0.5f : CR_Z - CR_FLEN * 0.5f };
            glm::vec3 toGoal = glm::normalize(targetGoal - B.pos);

            float kickPower = 8.f + static_cast<float>(rand() % 5);
            B.vel = toGoal * kickPower;
            B.vel.y = 3.5f + static_cast<float>(rand() % 3);  // loft
        }
    }
}

// ─── Static / cinematic fallback (used when match is NOT active) ──────────────
static void renderCinematicBall(unsigned int sh) {
    float t = (float)glfwGetTime();
    const float BALL_R = 0.22f;
    const float GROUND = CR_Y + BALL_R;
    float phase = fmodf(t, CR_CYCLE) / CR_CYCLE;
    float strikerZ = CR_Z - CR_FLEN * 0.3f + phase * CR_FLEN * 0.6f;
    float ballX, ballY, ballZ;
    if (phase < 0.75f) {
        ballX = CR_X - 2.f + 0.3f * sinf(t * 4.2f);
        ballZ = strikerZ + 0.5f;
        ballY = GROUND + fabsf(sinf(t * 9.5f)) * 0.18f;
    }
    else {
        float p = (phase - 0.75f) / 0.25f;
        float goalZ = CR_Z + CR_FLEN * 0.5f - 0.5f;
        ballX = CR_X - 2.f + p * 2.4f;
        ballZ = strikerZ + p * (goalZ - strikerZ);
        ballY = GROUND + 4.f * p * (1.f - p) * 4.2f;
    }
    float spinAngle = t * 480.f;
    glm::mat4 base = glm::rotate(glm::translate(glm::mat4(1.f), { ballX, ballY, ballZ }),
        glm::radians(spinAngle), { 1.f, 0.f, 0.f });
    glm::mat4 m = glm::scale(base, { BALL_R, BALL_R, BALL_R });
    drawSphere(sh, m, glm::vec3(0.96f, 0.96f, 0.94f), texMarble, 1.f);
    for (int i = 0; i < 6; i++) {
        float ang = i * 60.f;
        glm::mat4 p2 = glm::rotate(base, glm::radians(ang), { 0.f, 1.f, 0.f });
        p2 = glm::translate(p2, { 0.f, BALL_R, 0.f });
        m = glm::scale(p2, { 0.12f, 0.03f, 0.12f });
        drawCylinder(sh, m, glm::vec3(0.08f, 0.08f, 0.08f));
    }
    float shadowSc = BALL_R + 0.08f * (1.f - std::min((ballY - GROUND) / 4.2f, 1.f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { ballX, CR_Y + 0.005f, ballZ }),
        { shadowSc * 2.5f, 0.015f, shadowSc * 2.5f });
    drawCylinder(sh, m, glm::vec3(0.04f, 0.09f, 0.04f));
}

// ─── Draw ball from physics state ────────────────────────────────────────────
static void renderPhysicsBall(unsigned int sh) {
    const FootballBall& B = gFootballMatch.ball;
    const float R = B.radius;
    glm::mat4 base = glm::rotate(glm::translate(glm::mat4(1.f), B.pos),
        glm::radians(B.spinAngle), { 1.f, 0.f, 0.f });
    glm::mat4 m = glm::scale(base, { R, R, R });
    drawSphere(sh, m, glm::vec3(0.96f, 0.96f, 0.94f), texMarble, 1.f);
    for (int i = 0; i < 6; i++) {
        glm::mat4 p2 = glm::rotate(base, glm::radians(i * 60.f), { 0.f, 1.f, 0.f });
        p2 = glm::translate(p2, { 0.f, R, 0.f });
        m = glm::scale(p2, { 0.12f, 0.03f, 0.12f });
        drawCylinder(sh, m, glm::vec3(0.08f, 0.08f, 0.08f));
    }
    float normH = (B.pos.y - B.radius) / 4.f;
    float sc = R + 0.08f * (1.f - glm::clamp(normH, 0.f, 1.f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { B.pos.x, CR_Y + 0.005f, B.pos.z }),
        { sc * 2.5f, 0.015f, sc * 2.5f });
    drawCylinder(sh, m, glm::vec3(0.04f, 0.09f, 0.04f));
}

// ─── Draw a footballer (shared by cinematic & physics paths) ─────────────────
static void drawFootballerAt(unsigned int sh, float px, float pz,
    const glm::vec3& shirt, const glm::vec3& shrt,
    float legAngle1, float legAngle2, float baseY) {
    glm::mat4 m;
    const glm::vec3 SKIN(0.88f, 0.70f, 0.55f);
    const glm::vec3 SHOE(0.12f, 0.10f, 0.10f);
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { px + s * 0.16f, baseY + 0.12f, pz }), { 0.22f, 0.22f, 0.40f });
        drawCube(sh, m, SHOE);
    }
    for (int side : {-1, 1}) {
        float ang = (side == -1) ? legAngle1 : legAngle2;
        glm::mat4 leg = glm::rotate(glm::translate(glm::mat4(1.f), { px + side * 0.15f, baseY + 0.68f, pz }),
            glm::radians(ang), { 1.f, 0.f, 0.f });
        m = glm::scale(leg, { 0.22f, 0.82f, 0.22f }); drawCube(sh, m, shrt);
    }
    m = glm::scale(glm::translate(glm::mat4(1.f), { px, baseY + 1.60f, pz }), { 0.54f, 0.72f, 0.34f });
    drawCube(sh, m, shirt, texConcrete, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { px, baseY + 2.22f, pz }), { 0.42f, 0.42f, 0.42f });
    drawSphere(sh, m, SKIN, texMarble, 1.f);
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { px + s * 0.38f, baseY + 1.65f, pz }), { 0.14f, 0.55f, 0.14f });
        drawCube(sh, m, shirt);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  PUBLIC API
// ═══════════════════════════════════════════════════════════════════════════════

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

    // ── Score display (3D digit-block numerals near scoreboard) ───────────────
    if (gFootballMatch.active) {
        const FootballMatch& FM = gFootballMatch;
        // Simple score cubes: Team A (blue) and Team B (red) stacked blocks
        auto drawDigit = [&](int val, float ox, float oy, float oz, glm::vec3 col) {
            val = glm::clamp(val, 0, 9);
            for (int bit = 0; bit < 4; bit++) {
                if ((val >> bit) & 1) {
                    m = glm::scale(glm::translate(glm::mat4(1.f), { ox, oy + bit * 0.55f, oz }),
                        { 0.45f, 0.45f, 0.15f });
                    drawCube(sh, m, col);
                }
            }
            };
        float sx0 = sbX + 0.25f, sy0 = 6.5f, sz0 = sbZ - 2.8f;
        drawDigit(FM.scoreA, sx0, sy0, sz0, glm::vec3(0.20f, 0.35f, 0.90f));
        drawDigit(FM.scoreB, sx0 + 0.7f, sy0, sz0, glm::vec3(0.90f, 0.22f, 0.22f));
        // dash between scores
        m = glm::scale(glm::translate(glm::mat4(1.f), { sx0 + 0.35f, sy0 + 1.0f, sz0 }), { 0.3f, 0.12f, 0.12f });
        drawCube(sh, m, glm::vec3(0.9f, 0.9f, 0.9f));
    }
}

void renderFootballPlayers(unsigned int sh) {
    float t = (float)glfwGetTime();

    if (gFootballMatch.active) {
        // ── Physics-driven players ────────────────────────────────────────────
        updateFootballPhysics(deltaTime);
        const FootballMatch& M = gFootballMatch;
        for (int i = 0; i < FootballMatch::NUM_PLAYERS; i++) {
            const FootballPlayer& P = M.players[i];
            glm::vec3 toBall = M.ball.pos - P.pos;
            float dist = glm::length(toBall);
            float legA = (dist > 0.5f) ? 35.f * sinf(t * 4.f + i) : 0.f;
            drawFootballerAt(sh, P.pos.x, P.pos.z, P.shirt, P.shorts, legA, -legA, CR_Y);
        }
    }
    else {
        // ── Original cinematic players ────────────────────────────────────────
        float phase = fmodf(t, CR_CYCLE) / CR_CYCLE;
        float strikerZ = CR_Z - CR_FLEN * 0.3f + phase * CR_FLEN * 0.6f;
        float strideA = 35.f * sinf(t * 4.f);
        const glm::vec3 SHIRT_A(0.18f, 0.32f, 0.82f), SHORTS_A(0.14f, 0.14f, 0.60f);
        const glm::vec3 SHIRT_B(0.85f, 0.20f, 0.20f), SHORTS_B(0.65f, 0.12f, 0.12f);
        const glm::vec3 GK_SHIRT(0.22f, 0.68f, 0.28f), GK_S(0.18f, 0.50f, 0.20f);
        float gkSway = 1.8f * sinf(t * 1.2f);
        drawFootballerAt(sh, CR_X - 2.f, strikerZ, SHIRT_A, SHORTS_A, strideA, -strideA, CR_Y);
        drawFootballerAt(sh, CR_X + gkSway, CR_Z - CR_FLEN * 0.5f + 0.8f, GK_SHIRT, GK_S, 0.f, 0.f, CR_Y);
        struct PP { float x, z; glm::vec3 sh; glm::vec3 s2; };
        PP pp[] = {
            { CR_X - 8.f, CR_Z - 8.f, SHIRT_A, SHORTS_A },
            { CR_X + 8.f, CR_Z - 8.f, SHIRT_A, SHORTS_A },
            { CR_X - 6.f, CR_Z + 5.f, SHIRT_B, SHORTS_B },
            { CR_X + 6.f, CR_Z + 5.f, SHIRT_B, SHORTS_B },
        };
        for (auto& p : pp) {
            float ls = 12.f * sinf(t * 1.5f + p.x);
            drawFootballerAt(sh, p.x, p.z, p.sh, p.s2, ls, -ls, CR_Y);
        }
    }
}

void renderFootball(unsigned int sh) {
    if (gFootballMatch.active) {
        renderPhysicsBall(sh);
    }
    else {
        renderCinematicBall(sh);
    }
}