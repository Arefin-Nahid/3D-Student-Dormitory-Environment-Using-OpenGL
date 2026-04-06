/*
 * Student Dormitory – Full Campus, OpenGL 3.3 Core Profile
 * KUET CSE 4128 Lab Assignment – Texture Mapping
 *
 * LAYOUT:
 *   LEFT  BUILDING  x=-22..-10  Student Living  (camera enters -> sees interior room)
 *   RIGHT BUILDING  x=+10..+22  Common Facilities
 *   COURTYARD       x=-10..+10  grass, benches, trees, staircase (RIGHT side)
 *   GATE            z=+28  cylindrical pillars, texPillar in mode 4
 *   TENNIS COURT    x=-52, z=-10   (LEFT)
 *   FOOTBALL FIELD  x=+52, z=-10  (RIGHT)
 *
 * TEXTURE MODES (T cycles 0->4):
 *   0 = Phong only (no texture)
 *   1 = Simple image texture
 *   2 = Vertex Gouraud blend with texture
 *   3 = Fragment Phong blend with texture
 *   4 = Pillar image texture (pillar.png applied to gate pillars)
 *
 * KEYBOARD:
 *   W/S/A/D  move camera       E/R=up/down   X/Y/Z=pitch/yaw/roll
 *   T        cycle texture (0-4)
 *   1/2/3    Dir/Point/Spot light   5/6/7 Amb/Diff/Spec   L=Day/Night
 *   G        toggle 4-grid/free     F=Orbit   I=Bird's Eye   P=Reset
 *   V+0..4   viewport switch        ESC=quit
 *   Walk INTO the left building to see the dormitory room automatically!
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "globals.h"
#include "shaders.h"
#include "textures.h"
#include "geometry.h"
#include "utils.h"
#include "input.h"
#include "scene.h"

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* mon = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(mon);
    SCR_WIDTH = mode->width;
    SCR_HEIGHT = mode->height;

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
        "Student Dormitory - KUET Full Campus", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    SCR_WIDTH = fbW; SCR_HEIGHT = fbH;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);

    unsigned int shader = createShaderProgram();
    setupCube(); setupCylinder(32); setupSphere(32, 32); setupCone(32);
    createAllTextures();

    // Load pillar image texture (used when T cycles to mode 4)
    texPillar = loadTextureFromFile("textures/pillar.png");

    updateCameraVectors();
    printInstructions();

    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "texSampler"), 0);

    // Scene centre for fixed camera views
    const glm::vec3 SC(0.f, 3.f, 0.f);

    while (!glfwWindowShouldClose(window)) {
        float cf = (float)glfwGetTime();
        deltaTime = cf - lastFrame; lastFrame = cf;
        processInput(window);

        // When camera is inside building use a narrower FOV for interior feel
        bool inside = cameraInsideBuilding();
        float fov = inside ? 75.f : 45.f;

        glClearColor(
            dayMode ? 0.52f : 0.04f,
            dayMode ? 0.80f : 0.04f,
            dayMode ? 0.92f : 0.10f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);
        setupLighting(shader);

        if (activeViewport == -1) {
            // ── 4-viewport split ──────────────────────────────────────────
            int HW = SCR_WIDTH / 2, HH = SCR_HEIGHT / 2;
            float asp = (float)HW / (float)HH;
            glm::mat4 proj = customPerspective(glm::radians(45.f), asp, 0.1f, 600.f);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

            // Top-Left: Isometric NW
            glViewport(0, HH, HW, HH);
            {
                glm::mat4 v = glm::lookAt(SC + glm::vec3(-55.f, 42.f, 62.f), SC, { 0,1,0 });
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }

            // Top-Right: Front view from south
            glViewport(HW, HH, HW, HH);
            {
                glm::mat4 v = glm::lookAt(SC + glm::vec3(0.f, 10.f, 72.f), SC, { 0,1,0 });
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }

            // Bottom-Left: Top-down aerial
            glViewport(0, 0, HW, HH);
            {
                glm::mat4 v = glm::lookAt(SC + glm::vec3(0.f, 110.f, 0.f), SC, { 0,0,-1 });
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }

            // Bottom-Right: Free/orbit/bird's eye
            glViewport(HW, 0, HW, HH);
            {
                glm::mat4 v;
                if (camera.birdsEyeMode)
                    v = glm::lookAt(SC + glm::vec3(0.f, 85.f, 0.f), SC, { 0,0,-1 });
                else if (camera.orbitMode)
                    v = glm::lookAt({ SC.x + sinf(cf) * 80.f,SC.y + 25.f,SC.z + cosf(cf) * 80.f }, SC, { 0,1,0 });
                else
                    v = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }

        }
        else {
            // ── Single fullscreen ──────────────────────────────────────────
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            glm::mat4 proj = customPerspective(glm::radians(fov),
                (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.05f, 600.f);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

            glm::mat4 v;
            if (activeViewport == 0)
                v = glm::lookAt(SC + glm::vec3(-55.f, 42.f, 62.f), SC, { 0,1,0 });
            else if (activeViewport == 1)
                v = glm::lookAt(SC + glm::vec3(0.f, 10.f, 72.f), SC, { 0,1,0 });
            else if (activeViewport == 2)
                v = glm::lookAt(SC + glm::vec3(0.f, 110.f, 0.f), SC, { 0,0,-1 });
            else {
                if (camera.birdsEyeMode)
                    v = glm::lookAt(SC + glm::vec3(0.f, 85.f, 0.f), SC, { 0,0,-1 });
                else if (camera.orbitMode)
                    v = glm::lookAt({ SC.x + sinf(cf) * 80.f,SC.y + 25.f,SC.z + cosf(cf) * 80.f }, SC, { 0,1,0 });
                else
                    v = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
            }
            glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
            renderScene(shader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}