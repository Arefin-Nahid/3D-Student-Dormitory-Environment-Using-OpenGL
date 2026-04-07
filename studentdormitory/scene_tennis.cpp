#include "scene_tennis.h"
#include "match_state.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <iostream>

// Tennis court: LEFT side of campus
const float BC_X = -52.f;
const float BC_Z = 0.f;
const float BC_Y = 0.06f;
const float BC_LEN = 23.77f;
const float BC_WID = 10.97f;
const float BC_SGL = 8.23f;

// ─── Player initialisation ────────────────────────────────────────────────────
void TennisMatch::initPlayers() {
    float baseline = HALF_LEN - 1.5f;
    playerS = { { COURT_X, 0.f, -baseline }, 0.f, true,  0.f, 35.f };
    playerN = { { COURT_X, 0.f,  baseline }, 0.f, false, 0.f, 35.f };
}

void TennisMatch::serveBall(bool southServes) {
    faultState = false;
    float dirZ = southServes ? 1.f : -1.f;
    ball.pos = { COURT_X, TennisBall::COURT_Y + 1.5f,
                 (southServes ? -HALF_LEN + 1.5f : HALF_LEN - 1.5f) };
    // Launch: fast diagonal + high enough to clear net
    ball.vel = { 0.4f * (static_cast<float>(rand() % 3) - 1.f),   // slight x drift
                 3.0f,                                              // upward launch
                 dirZ * 9.5f };                                    // across court
    ball.spinAngle = 0.f;
}

// ─── Physics update ───────────────────────────────────────────────────────────
static void updateTennisPhysics(float dt) {
    TennisMatch& M = gTennisMatch;
    if (!M.active || M.paused) return;

    M.matchTime += dt;

    // Reset-serve timer
    if (M.faultState) {
        M.resetTimer -= dt;
        if (M.resetTimer <= 0.f) {
            M.faultState = false;
            // South serves if score is even, north otherwise (simple alternation)
            M.serveBall((M.scoreS + M.scoreN) % 2 == 0);
        }
        return;
    }

    TennisBall& B = M.ball;

    // Gravity
    B.vel.y -= TennisBall::GRAVITY * dt;
    B.pos += B.vel * dt;
    B.spinAngle += glm::length(glm::vec2(B.vel.x, B.vel.z)) * 200.f * dt;

    // ── Court surface bounce ──────────────────────────────────────────────────
    if (B.pos.y < TennisBall::COURT_Y) {
        B.pos.y = TennisBall::COURT_Y;
        if (B.vel.y < 0.f) B.vel.y *= -TennisBall::BOUNCE_COR;
        B.vel.x *= TennisBall::FRICTION;
        B.vel.z *= TennisBall::FRICTION;
    }

    // ── Net collision (z ≈ 0, height 1.07) ───────────────────────────────────
    // Check crossing from south→north and north→south
    static float prevBallZ = B.pos.z;
    bool crossedNet = (prevBallZ < TennisBall::NET_Z && B.pos.z >= TennisBall::NET_Z) ||
        (prevBallZ > TennisBall::NET_Z && B.pos.z <= TennisBall::NET_Z);
    if (crossedNet && B.pos.y < TennisBall::NET_Y) {
        // Ball hit the net → fault
        B.vel.z *= -0.3f;
        B.vel.y = 1.5f;
        M.faultState = true; M.resetTimer = 1.5f;
    }
    prevBallZ = B.pos.z;

    // ── Side boundaries ───────────────────────────────────────────────────────
    float hWid = BC_WID * 0.5f;
    if (fabsf(B.pos.x - BC_X) > hWid) {
        B.vel.x *= -0.6f;
        B.pos.x = BC_X + glm::clamp(B.pos.x - BC_X, -hWid, hWid);
    }

    // ── Out / scoring ─────────────────────────────────────────────────────────
    float hLen = BC_LEN * 0.5f;
    if (B.pos.z > BC_Z + hLen + 0.5f) {
        // Ball past north baseline → South scores
        M.scoreS++;
        std::cout << "🎾 Point to South!  Score: " << M.scoreS << " - " << M.scoreN << std::endl;
        M.faultState = true; M.resetTimer = 2.0f;
    }
    else if (B.pos.z < BC_Z - hLen - 0.5f) {
        // Ball past south baseline → North scores
        M.scoreN++;
        std::cout << "🎾 Point to North!  Score: " << M.scoreS << " - " << M.scoreN << std::endl;
        M.faultState = true; M.resetTimer = 2.0f;
    }

    // ── Player AI ─────────────────────────────────────────────────────────────
    auto updatePlayer = [&](TennisPlayer& P) {
        P.hitCooldown -= dt;

        // Move laterally to track ball X
        float targetX = BC_X + glm::clamp(B.pos.x - BC_X, -TennisPlayer::SWAY_LIMIT, TennisPlayer::SWAY_LIMIT);
        float dx = targetX - P.pos.x;
        float step = TennisPlayer::SPEED * dt;
        P.pos.x += glm::clamp(dx, -step, step);

        // Keep on own baseline
        float baseZ = P.isSouth ? -(BC_LEN * 0.5f - 1.5f) : (BC_LEN * 0.5f - 1.5f);
        P.pos.z = BC_Z + baseZ;

        // Check if ball is close enough to hit
        glm::vec3 toBall = B.pos - P.pos;
        float dist = glm::length(toBall);

        // Only hit if ball is on the player's side of the net
        bool ballOnMySide = P.isSouth ? (B.pos.z < TennisBall::NET_Z) : (B.pos.z > TennisBall::NET_Z);

        if (dist < TennisPlayer::HIT_RANGE && P.hitCooldown <= 0.f && ballOnMySide) {
            P.hitCooldown = TennisPlayer::HIT_CD;

            // Hit toward opposite court: target somewhere inside opponent's half
            float targetZ = P.isSouth ? BC_Z + BC_LEN * 0.3f : BC_Z - BC_LEN * 0.3f;
            float targetX2 = BC_X + (static_cast<float>(rand() % 7) - 3.f);
            glm::vec3 toTarget = glm::normalize(glm::vec3(targetX2, 0.f, targetZ) - glm::vec3(P.pos.x, 0.f, P.pos.z));
            float power = 9.f + static_cast<float>(rand() % 4);
            B.vel = toTarget * power;
            B.vel.y = 4.5f;  // clear net height

            // Animate arm
            P.armAngle = 80.f;
        }
        else {
            // Arm return animation
            P.armAngle = glm::mix(P.armAngle, 35.f, dt * 3.f);
        }
        };

    updatePlayer(M.playerS);
    updatePlayer(M.playerN);
}

// ─── Draw a tennis player ─────────────────────────────────────────────────────
static void drawTennisPlayerAt(unsigned int sh,
    float px, float pz,
    const glm::vec3& shirt, const glm::vec3& shorts,
    float armAngle, float t) {
    const glm::vec3 SKIN(0.88f, 0.70f, 0.55f);
    const glm::vec3 SHOE(0.15f, 0.12f, 0.10f);
    const glm::vec3 RACKET(0.25f, 0.22f, 0.22f);
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

    glm::mat4 armBase = glm::rotate(
        glm::translate(glm::mat4(1.f), { px + 0.40f, BY + 1.65f, pz }),
        glm::radians(armAngle), { 1.f, 0.f, 0.f });
    m = glm::scale(armBase, { 0.14f, 0.52f, 0.14f }); drawCube(sh, m, SKIN);

    glm::mat4 forearmBase = glm::rotate(
        glm::translate(armBase, { 0.f, 0.52f, 0.f }),
        glm::radians(armAngle * 0.5f), { 1.f, 0.f, 0.f });
    m = glm::scale(forearmBase, { 0.13f, 0.46f, 0.13f }); drawCube(sh, m, SKIN);

    glm::mat4 handleBase = glm::translate(forearmBase, { 0.f, 0.46f, 0.f });
    m = glm::scale(handleBase, { 0.07f, 0.50f, 0.07f });
    drawCylinder(sh, m, glm::vec3(0.40f, 0.30f, 0.18f));

    glm::mat4 racketRoot = glm::translate(handleBase, { 0.f, 0.52f, 0.f });
    m = glm::scale(racketRoot, { 0.22f, 0.22f, 0.05f }); drawCube(sh, m, RACKET);
    glm::mat4 racketHead = glm::translate(racketRoot, { 0.f, 0.30f, 0.f });
    m = glm::scale(racketHead, { 0.55f, 0.68f, 0.045f }); drawCube(sh, m, RACKET);
    m = glm::scale(racketHead, { 0.44f, 0.58f, 0.050f }); drawCube(sh, m, glm::vec3(0.90f, 0.90f, 0.85f));
}

// ═══════════════════════════════════════════════════════════════════════════════
//  PUBLIC API
// ═══════════════════════════════════════════════════════════════════════════════

void renderTennisCourt(unsigned int sh) {
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, BC_Y - 0.03f, BC_Z }), { BC_WID + 3.f, 0.08f, BC_LEN + 3.f });
    drawCube(sh, m, glm::vec3(0.12f, 0.28f, 0.55f), texConcrete, 4.f);

    const float LT = 0.12f, LH = 0.04f, SY = BC_Y + 0.02f;
    const glm::vec3 W(0.96f, 0.96f, 0.96f);

    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * BC_WID * 0.5f, SY, BC_Z }), { LT, LH, BC_LEN });
        drawCube(sh, m, W);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * BC_SGL * 0.5f, SY, BC_Z }), { LT, LH, BC_LEN });
        drawCube(sh, m, W);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, SY, BC_Z + s * BC_LEN * 0.5f }), { BC_WID, LH, LT });
        drawCube(sh, m, W);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, SY, BC_Z + s * 6.4f }), { BC_SGL, LH, LT });
        drawCube(sh, m, W);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, SY, BC_Z + s * 3.2f }), { LT, LH, 6.4f });
        drawCube(sh, m, W);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, SY, BC_Z + s * (BC_LEN * 0.5f - 0.2f) }), { LT, LH, 0.4f });
        drawCube(sh, m, W);
    }

    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * (BC_WID * 0.5f + 0.25f), 0.914f * 0.5f + BC_Y, BC_Z }), { 0.12f, 0.914f + BC_Y, 0.12f });
        drawCylinder(sh, m, glm::vec3(0.75f, 0.72f, 0.65f));
    }
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, BC_Y + 0.50f, BC_Z }), { BC_WID + 0.5f, 0.90f, 0.08f });
    drawCube(sh, m, glm::vec3(0.88f, 0.88f, 0.88f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, BC_Y + 0.96f, BC_Z }), { BC_WID + 0.5f, 0.08f, 0.12f });
    drawCube(sh, m, W);
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, BC_Y + 0.50f, BC_Z }), { 0.08f, 0.90f, 0.10f });
    drawCube(sh, m, W);

    const glm::vec3 FENCE(0.15f, 0.45f, 0.18f);
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + s * (BC_WID * 0.5f + 1.5f), 1.0f, BC_Z }), { 0.2f, 2.0f, BC_LEN + 5.f });
        drawCube(sh, m, FENCE, texGrass, 3.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, 1.0f, BC_Z + s * (BC_LEN * 0.5f + 1.5f) }), { BC_WID + 3.f, 2.0f, 0.2f });
        drawCube(sh, m, FENCE, texGrass, 3.f);
    }

    const glm::vec3 STEEL(0.50f, 0.50f, 0.52f);
    float pOX = BC_WID * 0.5f + 2.5f, pOZ = BC_LEN * 0.5f + 2.5f;
    for (int sx : {-1, 1}) for (int sz : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + sx * pOX, 6.f, BC_Z + sz * pOZ }), { 0.18f, 12.f, 0.18f });
        drawCylinder(sh, m, STEEL);
        m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + sx * pOX, 12.2f, BC_Z + sz * pOZ }), { 0.5f, 0.28f, 0.8f });
        drawCube(sh, m, glm::vec3(0.90f, 0.88f, 0.22f));
        if (!dayMode) {
            glm::vec3 lemit(0.85f, 0.85f, 0.10f), zero(0.f, 0.f, 0.f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(lemit));
            m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X + sx * pOX, 11.6f, BC_Z + sz * pOZ }), { 1.2f, 1.2f, 1.2f });
            drawCone(sh, m, glm::vec3(0.95f, 0.90f, 0.50f));
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(zero));
        }
    }

    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, 2.8f, BC_Z - BC_LEN * 0.5f - 1.6f }), { 4.0f, 0.8f, 0.15f });
    drawCube(sh, m, glm::vec3(0.12f, 0.12f, 0.45f));
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, 2.8f, BC_Z - BC_LEN * 0.5f - 1.55f }), { 3.6f, 0.55f, 0.08f });
    drawCube(sh, m, glm::vec3(0.95f, 0.95f, 0.92f));

    // ── Score display ─────────────────────────────────────────────────────────
    if (gTennisMatch.active) {
        const TennisMatch& TM = gTennisMatch;
        float sx0 = BC_X - 1.8f;
        float sy0 = 3.5f;
        float sz0 = BC_Z - BC_LEN * 0.5f - 1.55f;
        auto drawDigit = [&](int val, float ox, glm::vec3 col) {
            val = glm::clamp(val, 0, 9);
            for (int bit = 0; bit < 4; bit++) {
                if ((val >> bit) & 1) {
                    m = glm::scale(glm::translate(glm::mat4(1.f), { ox, sy0 + bit * 0.45f, sz0 - 0.1f }),
                        { 0.35f, 0.35f, 0.12f });
                    drawCube(sh, m, col);
                }
            }
            };
        drawDigit(TM.scoreS, sx0, glm::vec3(0.95f, 0.92f, 0.18f));
        drawDigit(TM.scoreN, sx0 + 0.5f, glm::vec3(0.90f, 0.22f, 0.22f));
    }
}

void renderTennisPlayers(unsigned int sh) {
    float t = (float)glfwGetTime();

    if (gTennisMatch.active) {
        // Physics-driven players (AI update happens inside renderTennisBall)
        const TennisMatch& M = gTennisMatch;
        const glm::vec3 SHIRT1(0.95f, 0.95f, 0.92f), SHORTS1(0.85f, 0.85f, 0.82f);
        const glm::vec3 SHIRT2(0.92f, 0.22f, 0.18f), SHORTS2(0.20f, 0.20f, 0.22f);
        drawTennisPlayerAt(sh, M.playerS.pos.x, M.playerS.pos.z, SHIRT1, SHORTS1, M.playerS.armAngle, t);
        drawTennisPlayerAt(sh, M.playerN.pos.x, M.playerN.pos.z, SHIRT2, SHORTS2, M.playerN.armAngle, t);
    }
    else {
        // Original cinematic sway
        float sway1 = 0.6f * sinf(t * 1.0f);
        float sway2 = -0.6f * sinf(t * 1.0f);
        float baselineOffset = BC_LEN * 0.5f - 1.5f;
        const glm::vec3 SHIRT1(0.95f, 0.95f, 0.92f), SHORTS1(0.85f, 0.85f, 0.82f);
        const glm::vec3 SHIRT2(0.92f, 0.22f, 0.18f), SHORTS2(0.20f, 0.20f, 0.22f);
        float armAngle1 = 35.f + 20.f * sinf(t * 1.0f);
        float armAngle2 = 35.f + 20.f * sinf(t * 1.0f + PI);
        drawTennisPlayerAt(sh, BC_X + sway1, BC_Z - baselineOffset, SHIRT1, SHORTS1, armAngle1, t);
        drawTennisPlayerAt(sh, BC_X + sway2, BC_Z + baselineOffset, SHIRT2, SHORTS2, armAngle2, t);
    }
}

void renderTennisBall(unsigned int sh) {
    float t = (float)glfwGetTime();

    if (gTennisMatch.active) {
        // ── Physics ball ──────────────────────────────────────────────────────
        updateTennisPhysics(deltaTime);
        const TennisBall& B = gTennisMatch.ball;

        glm::mat4 base = glm::rotate(glm::translate(glm::mat4(1.f), B.pos),
            glm::radians(B.spinAngle), { 1.f, 0.f, 0.f });
        glm::mat4 m = glm::scale(base, { B.radius, B.radius, B.radius });
        drawSphere(sh, m, glm::vec3(0.75f, 0.85f, 0.12f), texGrass, 1.f);

        glm::mat4 seam = glm::scale(glm::rotate(base, glm::radians(45.f), { 0.f, 0.f, 1.f }),
            { B.radius * 0.9f, 0.018f, B.radius * 0.9f });
        drawCylinder(sh, seam, glm::vec3(0.95f, 0.95f, 0.92f));

        float normY = (B.pos.y - TennisBall::COURT_Y) / 3.2f;
        float sc = 0.20f + 0.14f * (1.f - glm::clamp(normY, 0.f, 1.f));
        m = glm::scale(glm::translate(glm::mat4(1.f), { B.pos.x, BC_Y + 0.01f, B.pos.z }),
            { sc * 2.5f, 0.02f, sc * 2.5f });
        drawCylinder(sh, m, glm::vec3(0.04f, 0.08f, 0.18f));
    }
    else {
        // ── Original cinematic ball ───────────────────────────────────────────
        const float SPEED = 8.0f;
        const float COURT_Y_C = BC_Y + 0.09f;
        const float HALF_LEN = BC_LEN * 0.5f - 0.5f;
        float tripTime = HALF_LEN / SPEED;
        float cyclePeriod = tripTime * 2.0f;
        float ct = fmodf(t, cyclePeriod);
        float phase, dirZ;
        if (ct < tripTime) { phase = ct / tripTime; dirZ = 1.f; }
        else { phase = (ct - tripTime) / tripTime; dirZ = -1.f; }
        float ballZ = BC_Z + dirZ * (phase - 0.5f) * (2.f * HALF_LEN);
        float rawY = sinf(PI * phase * 2.f);
        float ballY = COURT_Y_C + fabsf(rawY) * 3.2f;
        float ballX = BC_X + 0.4f * sinf(t * 1.5f);
        float spinAngle = t * 540.f;
        glm::mat4 base = glm::rotate(glm::translate(glm::mat4(1.f), { ballX, ballY, ballZ }),
            glm::radians(spinAngle), { 1.f, 0.f, 0.f });
        glm::mat4 m = glm::scale(base, { 0.18f, 0.18f, 0.18f });
        drawSphere(sh, m, glm::vec3(0.75f, 0.85f, 0.12f), texGrass, 1.f);
        glm::mat4 seam = glm::scale(glm::rotate(base, glm::radians(45.f), { 0.f, 0.f, 1.f }), { 0.165f, 0.02f, 0.165f });
        drawCylinder(sh, seam, glm::vec3(0.95f, 0.95f, 0.92f));
        float normY2 = (ballY - COURT_Y_C) / 3.2f;
        float shadowSc = 0.20f + 0.14f * (1.f - normY2);
        m = glm::scale(glm::translate(glm::mat4(1.f), { ballX, BC_Y + 0.01f, ballZ }),
            { shadowSc * 2.5f, 0.02f, shadowSc * 2.5f });
        drawCylinder(sh, m, glm::vec3(0.04f, 0.08f, 0.18f));
    }
}