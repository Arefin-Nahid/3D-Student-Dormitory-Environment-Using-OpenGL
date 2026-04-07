#include "input.h"
#include "globals.h"
#include "utils.h"
#include "match_state.h"
#include <iostream>

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float vel = camera.speed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.position += camera.front * vel;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.position -= camera.front * vel;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.position -= camera.right * vel;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.position += camera.right * vel;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.position += camera.up * vel;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) camera.position -= camera.up * vel;

    // ── Bidirectional rotation with SHIFT modifier ────────────────────────────
    // Detect either SHIFT key being held
    bool shiftHeld = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    const float ROT_SPEED = 60.f * deltaTime; // degrees per second

    // X → Pitch up (+) | SHIFT+X → Pitch down (−)
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        camera.pitch += shiftHeld ? -ROT_SPEED : ROT_SPEED;
        updateCameraVectors();
    }
    // Y → Yaw right (+) | SHIFT+Y → Yaw left (−)
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        camera.yaw += shiftHeld ? -ROT_SPEED : ROT_SPEED;
        updateCameraVectors();
    }
    // Z → Roll clockwise (+) | SHIFT+Z → Roll counterclockwise (−)
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        camera.roll += shiftHeld ? -ROT_SPEED : ROT_SPEED;
        updateCameraVectors();
    }

    // G: toggle 4-grid / single free camera
    static bool gP = false;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gP) {
        activeViewport = (activeViewport == -1) ? 3 : -1;  gP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) gP = false;

    // P: reset camera
    static bool pP = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pP) {
        camera.position = glm::vec3(0.f, 12.f, 65.f);
        camera.yaw = -90.f; camera.pitch = -10.f; camera.roll = 0.f;
        updateCameraVectors(); pP = true;
        std::cout << "[P] Camera reset." << std::endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) pP = false;

    // F: orbit mode
    static bool fP = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fP) {
        camera.orbitMode = !camera.orbitMode; fP = true;
        std::cout << "Orbit: " << (camera.orbitMode ? "ON" : "OFF") << std::endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) fP = false;

    // I: bird's eye
    static bool iP = false;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS && !iP) {
        camera.birdsEyeMode = !camera.birdsEyeMode; iP = true;
        std::cout << "Bird's Eye: " << (camera.birdsEyeMode ? "ON" : "OFF") << std::endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE) iP = false;

    // T: cycle texture mode 0-4
    static bool tP = false;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !tP) {
        textureMode = (textureMode + 1) % 5;
        const char* names[] = {
            "0 No Texture (Phong)",
            "1 Simple Texture",
            "2 Vertex Gouraud + Texture",
            "3 Fragment Phong + Texture",
            "4 Pillar Image Texture (pillar.png)"
        };
        std::cout << "[T] Texture Mode -> " << names[textureMode] << std::endl;
        tP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) tP = false;

    // ── M: Toggle Football Match (Start / Pause / Reset) ─────────────────────
    static bool mP = false;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !mP) {
        mP = true;
        if (!gFootballMatch.active) {
            gFootballMatch.reset();
            gFootballMatch.active = true;
            gFootballMatch.paused = false;
            std::cout << "[M] Football Match STARTED  (Score 0-0)" << std::endl;
        }
        else if (!gFootballMatch.paused) {
            gFootballMatch.paused = true;
            std::cout << "[M] Football Match PAUSED" << std::endl;
        }
        else {
            gFootballMatch.paused = false;
            std::cout << "[M] Football Match RESUMED" << std::endl;
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) mP = false;

    // SHIFT+M → Reset Football Match
    static bool smP = false;
    if (shiftHeld && glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !smP) {
        smP = true;
        gFootballMatch.reset();
        std::cout << "[SHIFT+M] Football Match RESET" << std::endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) smP = false;

    // ── N: Toggle Tennis Match (Start / Pause / Reset) ───────────────────────
    static bool nP = false;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !nP) {
        nP = true;
        if (!gTennisMatch.active) {
            gTennisMatch.reset();
            gTennisMatch.active = true;
            gTennisMatch.paused = false;
            std::cout << "[N] Tennis Match STARTED  (Score 0-0)" << std::endl;
        }
        else if (!gTennisMatch.paused) {
            gTennisMatch.paused = true;
            std::cout << "[N] Tennis Match PAUSED" << std::endl;
        }
        else {
            gTennisMatch.paused = false;
            std::cout << "[N] Tennis Match RESUMED" << std::endl;
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) nP = false;

    // SHIFT+N → Reset Tennis Match
    static bool snP = false;
    if (shiftHeld && glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !snP) {
        snP = true;
        gTennisMatch.reset();
        std::cout << "[SHIFT+N] Tennis Match RESET" << std::endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) snP = false;

#define TOGGLE(key,var,name) \
    { static bool p=false; \
      if (glfwGetKey(window,key)==GLFW_PRESS&&!p){var=!var;p=true;std::cout<<name<<": "<<(var?"ON":"OFF")<<std::endl;} \
      else if (glfwGetKey(window,key)==GLFW_RELEASE) p=false; }
    TOGGLE(GLFW_KEY_1, lighting.directionalLightOn, "Dir Light")
        TOGGLE(GLFW_KEY_2, lighting.pointLightsOn, "Point Lights")
        TOGGLE(GLFW_KEY_3, lighting.spotLightOn, "Spot Light")
        TOGGLE(GLFW_KEY_5, lighting.ambientOn, "Ambient")
        TOGGLE(GLFW_KEY_6, lighting.diffuseOn, "Diffuse")
        TOGGLE(GLFW_KEY_7, lighting.specularOn, "Specular")
        TOGGLE(GLFW_KEY_L, dayMode, "Day/Night")
#undef TOGGLE

        static bool v0P = false, v1P = false, v2P = false, v3P = false, v4P = false;
    bool vHeld = (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS);
    if (vHeld && glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS && !v0P) { activeViewport = -1; v0P = true; std::cout << "[V+0] 4-Viewport" << std::endl; }
    else if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE) v0P = false;
    if (vHeld && glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !v1P) { activeViewport = 0; v1P = true; std::cout << "[V+1] Isometric" << std::endl; }
    else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) v1P = false;
    if (vHeld && glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !v2P) { activeViewport = 1; v2P = true; std::cout << "[V+2] Front View" << std::endl; }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) v2P = false;
    if (vHeld && glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !v3P) { activeViewport = 2; v3P = true; std::cout << "[V+3] Top-Down" << std::endl; }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) v3P = false;
    if (vHeld && glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !v4P) { activeViewport = 3; v4P = true; std::cout << "[V+4] Free Camera" << std::endl; }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) v4P = false;
}

void printInstructions() {
    std::cout << "\n+==================================================================+" << std::endl;
    std::cout << "|       STUDENT DORMITORY - KUET  (Full Campus Simulation)        |" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  TEXTURE MODES (T cycles 0->4):                                 |" << std::endl;
    std::cout << "|   0 = No Texture (Phong)         3 = Fragment Phong + Texture   |" << std::endl;
    std::cout << "|   1 = Simple Texture             4 = Pillar image (pillar.png)  |" << std::endl;
    std::cout << "|   2 = Vertex Gouraud + Texture                                  |" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  LIGHTING:  1=Dir  2=Point  3=Spot  5=Amb  6=Diff  7=Spec  L=Day|" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  CAMERA: W/S/A/D=move  E/R=up/dn                                |" << std::endl;
    std::cout << "|    Rotation: X=pitch+  SHIFT+X=pitch-                           |" << std::endl;
    std::cout << "|              Y=yaw+    SHIFT+Y=yaw-                             |" << std::endl;
    std::cout << "|              Z=roll+   SHIFT+Z=roll-                            |" << std::endl;
    std::cout << "|    F=Orbit  I=Bird's Eye  P=Reset  G=Grid toggle               |" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  SPORTS:  M=Football Match (Start/Pause)  SHIFT+M=Reset         |" << std::endl;
    std::cout << "|           N=Tennis Match   (Start/Pause)  SHIFT+N=Reset         |" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  VIEWPORTS: V+0=4split V+1=Iso V+2=Front V+3=Top V+4=Free       |" << std::endl;
    std::cout << "|  ESC = Quit                                                      |" << std::endl;
    std::cout << "+==================================================================+\n" << std::endl;
}