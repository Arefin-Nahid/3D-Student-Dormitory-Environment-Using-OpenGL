#include "utils.h"
#include "globals.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <cmath>

void updateCameraVectors() {
    glm::vec3 front;
    front.x = cosf(glm::radians(camera.yaw)) * cosf(glm::radians(camera.pitch));
    front.y = sinf(glm::radians(camera.pitch));
    front.z = sinf(glm::radians(camera.yaw)) * cosf(glm::radians(camera.pitch));
    camera.front = glm::normalize(front);
    camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
    camera.up = glm::normalize(glm::cross(camera.right, camera.front));
}

glm::mat4 customPerspective(float fovY, float aspect, float zN, float zF) {
    float t = tanf(fovY / 2.f); glm::mat4 r(0.f);
    r[0][0] = 1.f / (aspect * t); r[1][1] = 1.f / t;
    r[2][2] = -(zF + zN) / (zF - zN); r[2][3] = -1.f;
    r[3][2] = -(2.f * zF * zN) / (zF - zN);
    return r;
}

void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    SCR_WIDTH = w; SCR_HEIGHT = h; glViewport(0, 0, w, h);
}

std::string itos(int v) { std::ostringstream ss; ss << v; return ss.str(); }

void setupLighting(unsigned int sh) {
    glUniform1i(glGetUniformLocation(sh, "directionalLightOn"), lighting.directionalLightOn);
    glUniform1i(glGetUniformLocation(sh, "pointLightsOn"), lighting.pointLightsOn);
    glUniform1i(glGetUniformLocation(sh, "spotLightOn"), lighting.spotLightOn);
    glUniform1i(glGetUniformLocation(sh, "ambientOn"), lighting.ambientOn);
    glUniform1i(glGetUniformLocation(sh, "diffuseOn"), lighting.diffuseOn);
    glUniform1i(glGetUniformLocation(sh, "specularOn"), lighting.specularOn);
    glUniform3f(glGetUniformLocation(sh, "viewPos"),
        camera.position.x, camera.position.y, camera.position.z);

    if (dayMode) {
        glUniform3f(glGetUniformLocation(sh, "dirLight_direction"), -0.5f, -1.0f, -0.4f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_ambient"), 0.32f, 0.30f, 0.28f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_diffuse"), 0.82f, 0.78f, 0.70f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_specular"), 1.00f, 1.00f, 0.90f);
    }
    else {
        glUniform3f(glGetUniformLocation(sh, "dirLight_direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_ambient"), 0.05f, 0.05f, 0.10f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_diffuse"), 0.10f, 0.10f, 0.20f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_specular"), 0.30f, 0.30f, 0.50f);
    }

    // 6 point lights
    // Slots 0-1: building exteriors, 2-3: courtyard lamp clusters,
    // 4-5: building interiors (corridor + common room)
    glm::vec3 ptPos[] = {
        {-16.f,  5.0f,  0.f},
        { 16.f,  5.0f,  0.f},
        {  0.f,  7.0f, 10.f},
        {  0.f,  7.0f,-10.f},
        {-16.f,  2.8f,  0.f},
        { 16.f,  2.8f,  0.f},
    };
    glm::vec3 ptCol[] = {
        {1.f,0.92f,0.72f},{0.9f,0.92f,1.0f},
        {1.f,0.95f,0.75f},{1.f,0.95f,0.75f},
        {1.f,0.97f,0.85f},{1.f,0.97f,0.85f},
    };
    float ptConst[] = { 1.f,1.f,1.f,1.f,1.f,1.f };
    float ptLinear[] = { 0.027f,0.027f,0.035f,0.035f,0.14f,0.14f };
    float ptQuad[] = { 0.0028f,0.0028f,0.004f,0.004f,0.07f,0.07f };

    // When camera is inside building, strengthen interior lights
    if (cameraInsideBuilding()) {
        ptPos[4] = { camera.position.x, camera.position.y + 1.5f, camera.position.z };
        ptCol[4] = { 1.0f, 0.97f, 0.88f };
        ptConst[4] = 1.f; ptLinear[4] = 0.20f; ptQuad[4] = 0.08f;
    }

    for (int i = 0; i < 6; i++) {
        std::string n = itos(i);
        glUniform3fv(glGetUniformLocation(sh, ("pointLights_position[" + n + "]").c_str()), 1, glm::value_ptr(ptPos[i]));
        glUniform3f(glGetUniformLocation(sh, ("pointLights_ambient[" + n + "]").c_str()),
            (i >= 4) ? 0.38f : 0.05f, (i >= 4) ? 0.38f : 0.05f, (i >= 4) ? 0.35f : 0.05f);
        glUniform3fv(glGetUniformLocation(sh, ("pointLights_diffuse[" + n + "]").c_str()), 1, glm::value_ptr(ptCol[i] * 0.72f));
        glUniform3fv(glGetUniformLocation(sh, ("pointLights_specular[" + n + "]").c_str()), 1, glm::value_ptr(ptCol[i]));
        glUniform1f(glGetUniformLocation(sh, ("pointLights_constant[" + n + "]").c_str()), ptConst[i]);
        glUniform1f(glGetUniformLocation(sh, ("pointLights_linear[" + n + "]").c_str()), ptLinear[i]);
        glUniform1f(glGetUniformLocation(sh, ("pointLights_quadratic[" + n + "]").c_str()), ptQuad[i]);
    }

    // ───────── PERFECT STAIR SPOTLIGHT ─────────

// Position: directly above the staircase center
    glUniform3f(glGetUniformLocation(sh, "spotLight_position"),
        0.0f,   // center X (stairs are centered)
        6.5f,   // height above stairs
        -11.5f  // middle of staircase (between -10 to -16)
    );

    // Direction: straight down + slightly toward building
    glm::vec3 dir = glm::normalize(glm::vec3(0.0f, -1.0f, -0.3f));
    glUniform3fv(glGetUniformLocation(sh, "spotLight_direction"), 1, glm::value_ptr(dir));

    // Narrow beam → focused on stairs only
    glUniform1f(glGetUniformLocation(sh, "spotLight_cutOff"),
        cosf(glm::radians(28.0f))
    );

    // Stronger inner beam (optional but realistic)
    glUniform1f(glGetUniformLocation(sh, "spotLight_outerCutOff"),
        cosf(glm::radians(38.0f))
    );

    // Light color (warm)
    glUniform3f(glGetUniformLocation(sh, "spotLight_ambient"), 0.05f, 0.05f, 0.05f);
    glUniform3f(glGetUniformLocation(sh, "spotLight_diffuse"), 1.0f, 0.95f, 0.80f);
    glUniform3f(glGetUniformLocation(sh, "spotLight_specular"), 1.0f, 0.95f, 0.80f);

    // Attenuation (important for realism)
    glUniform1f(glGetUniformLocation(sh, "spotLight_constant"), 1.0f);
    glUniform1f(glGetUniformLocation(sh, "spotLight_linear"), 0.09f);
    glUniform1f(glGetUniformLocation(sh, "spotLight_quadratic"), 0.032f);
}