#include "input.h"
#include "globals.h"
#include "utils.h"
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
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) { camera.pitch += 1.f; updateCameraVectors(); }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) { camera.yaw += 1.f; updateCameraVectors(); }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) { camera.roll += 1.f; updateCameraVectors(); }

    // G: toggle 4-grid / single free camera
    static bool gP = false;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gP) {
        activeViewport = (activeViewport == -1) ? 3 : -1;
        gP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) gP = false;

    // P: reset camera to overview
    static bool pP = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pP) {
        roomViewMode = false;
        camera.position = glm::vec3(0.f, 28.f, 75.f);
        camera.yaw = -90.f; camera.pitch = -18.f; camera.roll = 0.f;
        updateCameraVectors(); pP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) pP = false;

    // F: orbit mode
    static bool fP = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fP) {
        camera.orbitMode = !camera.orbitMode; fP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) fP = false;

    // I: bird's eye mode
    static bool iP = false;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS && !iP) {
        camera.birdsEyeMode = !camera.birdsEyeMode; iP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE) iP = false;

    // O: enter / leave dorm room (no console message – seamless transition)
    static bool oP = false;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !oP) {
        roomViewMode = !roomViewMode;
        if (roomViewMode) {
            // Place camera just inside the room door, looking into the room
            camera.position = glm::vec3(200.f, 1.72f, 208.2f);
            camera.yaw = -90.f;
            camera.pitch = -5.f;
            camera.roll = 0.f;
        }
        else {
            camera.position = glm::vec3(0.f, 28.f, 75.f);
            camera.yaw = -90.f;
            camera.pitch = -18.f;
            camera.roll = 0.f;
        }
        updateCameraVectors();
        oP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) oP = false;

    // T: cycle texture mode
    static bool tP = false;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !tP) {
        textureMode = (textureMode + 1) % 4; tP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) tP = false;

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
    if (vHeld && glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS && !v0P) { activeViewport = -1; v0P = true; }
    else if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE) v0P = false;
    if (vHeld && glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !v1P) { activeViewport = 0;  v1P = true; }
    else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) v1P = false;
    if (vHeld && glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !v2P) { activeViewport = 1;  v2P = true; }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) v2P = false;
    if (vHeld && glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !v3P) { activeViewport = 2;  v3P = true; }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) v3P = false;
    if (vHeld && glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !v4P) { activeViewport = 3;  v4P = true; }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) v4P = false;
}

void printInstructions() {
    std::cout << "\n+==================================================================+" << std::endl;
    std::cout << "|       STUDENT DORMITORY - KUET  (Full Campus Simulation)        |" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  TEXTURE MODES (T to cycle):                                    |" << std::endl;
    std::cout << "|   0 - No Texture (Phong only)    2 - Vertex Gouraud + Texture   |" << std::endl;
    std::cout << "|   1 - Simple Texture             3 - Fragment Phong + Texture   |" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  LIGHTING:  1=Dir  2=Point  3=Spot  5=Ambient  6=Diff  7=Spec   |" << std::endl;
    std::cout << "|             L = Day / Night toggle                               |" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  CAMERA: W/S/A/D=move  E/R=up/down  X/Y/Z=pitch/yaw/roll       |" << std::endl;
    std::cout << "|          F=Orbit  I=Bird's Eye  P=Reset overview                |" << std::endl;
    std::cout << "|          O=Enter room (walk inside)  O again=Exit room          |" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  VIEWPORTS: G=toggle grid/free  V+0..4=switch viewport          |" << std::endl;
    std::cout << "|   V+0 = 4-split  V+1 = Isometric  V+2 = Front  V+3 = Top       |" << std::endl;
    std::cout << "|   V+4 = Free Camera     ESC = Quit                              |" << std::endl;
    std::cout << "+==================================================================+\n" << std::endl;
}