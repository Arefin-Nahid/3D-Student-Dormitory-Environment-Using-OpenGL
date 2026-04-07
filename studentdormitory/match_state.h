#pragma once
#pragma once
#include <glm/glm.hpp>

// ─────────────────────────────────────────────────────────────────────────────
//  match_state.h
//  Shared physics state for the Football and Tennis interactive match systems.
//  Both scene files #include this header; globals.cpp defines the instances.
// ─────────────────────────────────────────────────────────────────────────────

// ── Football ─────────────────────────────────────────────────────────────────

struct FootballBall {
    glm::vec3 pos = { 52.f, 0.28f, 0.f };   // on ground at field centre
    glm::vec3 vel = { 0.f,  0.f,   0.f };
    float     radius = 0.22f;
    float     spinAngle = 0.f;

    // Physics constants
    static constexpr float GRAVITY = 9.8f;
    static constexpr float BOUNCE_COR = 0.65f;   // coefficient of restitution
    static constexpr float FRICTION = 0.92f;   // per-frame horizontal damping
    static constexpr float GROUND_Y = 0.f;     // field base Y
};

struct FootballPlayer {
    glm::vec3 pos;
    glm::vec3 vel = { 0.f, 0.f, 0.f };
    glm::vec3 shirt;
    glm::vec3 shorts;
    bool      isGoalkeeper = false;
    float     kickCooldown = 0.f;   // seconds until next kick is allowed

    static constexpr float SPEED = 5.5f;    // movement speed
    static constexpr float KICK_RANGE = 1.2f;    // distance at which kick triggers
    static constexpr float KICK_CD = 0.8f;    // cooldown between kicks (s)
};

struct FootballMatch {
    bool active = false;
    bool paused = false;
    int  scoreA = 0;  // Team A (blue)
    int  scoreB = 0;  // Team B (red)

    FootballBall ball;

    static constexpr int NUM_PLAYERS = 6;   // 3 per side (+ 1 GK each)
    FootballPlayer players[NUM_PLAYERS];

    // Timing
    float matchTime = 0.f;
    float resetTimer = 0.f;   // countdown after goal before re-kick
    bool  goalCelebration = false;

    // Goal bounds (derived from scene_football constants)
    static constexpr float GOAL_W = 5.0f;
    static constexpr float GOAL_H = 2.5f;
    static constexpr float FIELD_Z = 20.f;   // CR_FLEN * 0.5f
    static constexpr float FIELD_X = 52.f;   // CR_X
    static constexpr float FIELD_HW = 12.5f;  // CR_FWID * 0.5f

    void reset() {
        active = true; paused = false;
        scoreA = 0; scoreB = 0;
        matchTime = 0.f; resetTimer = 0.f;
        goalCelebration = false;
        resetBall();
        initPlayers();
    }

    void resetBall() {
        ball.pos = { FIELD_X, ball.radius, 0.f };
        // Kick-off: launch diagonally toward south goal
        ball.vel = { 1.2f, 3.5f, -6.5f };
        ball.spinAngle = 0.f;
    }

    void initPlayers();   // defined in scene_football.cpp
};

// ── Tennis ───────────────────────────────────────────────────────────────────

struct TennisBall {
    glm::vec3 pos = { -52.f, 1.5f, 0.f };
    glm::vec3 vel = { 0.f, 0.f, 0.f };
    float     radius = 0.09f;
    float     spinAngle = 0.f;

    static constexpr float GRAVITY = 12.0f;
    static constexpr float BOUNCE_COR = 0.80f;
    static constexpr float COURT_Y = 0.08f;  // BC_Y + radius
    static constexpr float NET_Y = 1.07f;  // top of net
    static constexpr float NET_Z = 0.f;    // net is at z=0
    static constexpr float FRICTION = 0.95f;
};

enum class TennisHitSide { South = 0, North = 1 };

struct TennisPlayer {
    glm::vec3 pos;
    float     sway = 0.f;    // lateral sway (-x to +x within court)
    bool      isSouth = true;
    float     hitCooldown = 0.f;
    float     armAngle = 35.f;

    static constexpr float SPEED = 4.5f;
    static constexpr float HIT_RANGE = 1.0f;
    static constexpr float HIT_CD = 0.6f;
    static constexpr float SWAY_LIMIT = 3.5f;  // max lateral distance from centre
};

struct TennisMatch {
    bool active = false;
    bool paused = false;
    int  scoreS = 0;  // South player
    int  scoreN = 0;  // North player

    TennisBall   ball;
    TennisPlayer playerS;  // South baseline
    TennisPlayer playerN;  // North baseline

    float matchTime = 0.f;
    float resetTimer = 0.f;
    bool  faultState = false;  // ball went out / hit net → reset serve

    static constexpr float COURT_X = -52.f;  // BC_X
    static constexpr float HALF_LEN = 11.885f; // BC_LEN * 0.5
    static constexpr float HALF_WID = 5.485f; // BC_WID * 0.5

    void reset() {
        active = true; paused = false;
        scoreS = 0; scoreN = 0;
        matchTime = 0.f; resetTimer = 0.f;
        faultState = false;
        initPlayers();
        serveBall(true);
    }

    void serveBall(bool southServes);
    void initPlayers();
};

// ── Global instances (defined in globals.cpp) ────────────────────────────────
extern FootballMatch gFootballMatch;
extern TennisMatch   gTennisMatch;