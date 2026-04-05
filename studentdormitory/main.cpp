/*
 * Student Dormitory – Full Campus, OpenGL 3.3 Core Profile
 * KUET CSE 4128 Lab Assignment – Texture Mapping
 *
 * LAYOUT: Two parallel buildings (Left=Student Living, Right=Common Facilities)
 *         flanking a central courtyard with staircase, trees, lamp posts.
 *
 * VIEWPORTS (4-split default):
 *   Top-Left     VP1 : Isometric – NW angle, sees both buildings + courtyard
 *   Top-Right    VP2 : Front View – looking north from z=+70 at campus centre
 *   Bottom-Left  VP3 : Top-Down aerial – directly above campus
 *   Bottom-Right VP4 : Free Camera (W/S/A/D/E/R to move)
 *
 * KEYBOARD:
 *   G        toggle 4-grid / single free-camera view
 *   T        cycle texture mode (0=Phong 1=Tex 2=Gouraud 3=PhongTex)
 *   1/2/3    Dir / Point / Spot light ON/OFF
 *   5/6/7    Ambient / Diffuse / Specular ON/OFF
 *   L        Day / Night
 *   W/S/A/D  camera forward/back/left/right
 *   E/R      camera up/down
 *   X/Y/Z    camera pitch/yaw/roll
 *   F        orbit mode
 *   I        bird's eye mode
 *   O        enter dormitory room (walk inside room 303)
 *   P        reset camera to overview
 *   V+0..4   viewport switching
 *   ESC      quit
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
        "Student Dormitory – KUET Full Campus", NULL, NULL);
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
    updateCameraVectors();
    printInstructions();

    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "texSampler"), 0);

    // Campus centre: two buildings at x=±16, courtyard at x=0, z range -16..+16
    // Scene centre for fixed cameras
    const glm::vec3 SC(0.f, 3.f, 0.f);

    while (!glfwWindowShouldClose(window)) {
        float cf = (float)glfwGetTime();
        deltaTime = cf - lastFrame; lastFrame = cf;
        processInput(window);

        // Sky colour changes day/night
        glClearColor(
            dayMode ? 0.52f : 0.04f,
            dayMode ? 0.80f : 0.04f,
            dayMode ? 0.92f : 0.10f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);
        setupLighting(shader);

        if (roomViewMode) {
            // ── Dorm room: fullscreen first-person ─────────────────────────
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            glm::mat4 proj = customPerspective(glm::radians(65.f),
                (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.05f, 300.f);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
            glm::mat4 v = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
            glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
            renderScene(shader);

        }
        else if (activeViewport == -1) {
            // ── 4-viewport split ───────────────────────────────────────────
            int HW = SCR_WIDTH / 2, HH = SCR_HEIGHT / 2;
            float asp = (float)HW / (float)HH;

            glm::mat4 proj = customPerspective(glm::radians(45.f), asp, 0.1f, 600.f);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

            // ── Top-Left: Isometric NW (shows both buildings + courtyard) ──
            glViewport(0, HH, HW, HH);
            {
                // Eye from north-west high angle looking at campus centre
                glm::vec3 eye = SC + glm::vec3(-55.f, 42.f, 62.f);
                glm::mat4 v = glm::lookAt(eye, SC, { 0,1,0 });
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }

            // ── Top-Right: Front View (from south, looking north at gate) ──
            glViewport(HW, HH, HW, HH);
            {
                glm::vec3 eye = SC + glm::vec3(0.f, 10.f, 72.f);
                glm::mat4 v = glm::lookAt(eye, SC, { 0,1,0 });
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }

            // ── Bottom-Left: Top-Down aerial ───────────────────────────────
            glViewport(0, 0, HW, HH);
            {
                glm::vec3 eye = SC + glm::vec3(0.f, 110.f, 0.f);
                glm::mat4 v = glm::lookAt(eye, SC, { 0, 0, -1 });
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }

            // ── Bottom-Right: Free / Orbit / Bird's Eye ────────────────────
            glViewport(HW, 0, HW, HH);
            {
                glm::mat4 v;
                if (camera.birdsEyeMode) {
                    glm::vec3 eye = SC + glm::vec3(0.f, 85.f, 0.f);
                    v = glm::lookAt(eye, SC, { 0, 0, -1 });
                }
                else if (camera.orbitMode) {
                    float ox = SC.x + sinf(cf) * 80.f;
                    float oz = SC.z + cosf(cf) * 80.f;
                    v = glm::lookAt({ ox, SC.y + 25.f, oz }, SC, { 0,1,0 });
                }
                else {
                    v = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
                }
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }

        }
        else {
            // ── Single fullscreen viewport ─────────────────────────────────
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            glm::mat4 proj = customPerspective(glm::radians(45.f),
                (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 600.f);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

            glm::mat4 v;
            if (activeViewport == 0) {
                // VP1: Isometric NW
                glm::vec3 eye = SC + glm::vec3(-55.f, 42.f, 62.f);
                v = glm::lookAt(eye, SC, { 0,1,0 });
            }
            else if (activeViewport == 1) {
                // VP2: Front view from south
                glm::vec3 eye = SC + glm::vec3(0.f, 10.f, 72.f);
                v = glm::lookAt(eye, SC, { 0,1,0 });
            }
            else if (activeViewport == 2) {
                // VP3: Top-down
                glm::vec3 eye = SC + glm::vec3(0.f, 110.f, 0.f);
                v = glm::lookAt(eye, SC, { 0,0,-1 });
            }
            else {
                // VP4: Free / orbit / bird's eye
                if (camera.birdsEyeMode) {
                    v = glm::lookAt(SC + glm::vec3(0.f, 85.f, 0.f), SC, { 0,0,-1 });
                }
                else if (camera.orbitMode) {
                    float ox = SC.x + sinf(cf) * 80.f;
                    float oz = SC.z + cosf(cf) * 80.f;
                    v = glm::lookAt({ ox, SC.y + 25.f, oz }, SC, { 0,1,0 });
                }
                else {
                    v = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
                }
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