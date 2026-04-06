#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// ─── Screen dimensions ───────────────────────────────────────────────────────
extern int SCR_WIDTH;
extern int SCR_HEIGHT;
extern const float PI;
extern int activeViewport;

// ─── Camera ──────────────────────────────────────────────────────────────────
struct Camera {
    glm::vec3 position{ 0.f, 12.f, 65.f };
    glm::vec3 front{ 0.f, -0.18f, -1.f };
    glm::vec3 up{ 0.f,  1.f,  0.f };
    glm::vec3 right{ 1.f,  0.f,  0.f };
    glm::vec3 worldUp{ 0.f,  1.f,  0.f };
    float yaw = -90.f, pitch = -10.f, roll = 0.f;
    float speed = 5.f;
    bool birdsEyeMode = false, orbitMode = false;
};

struct LightingState {
    bool directionalLightOn = true, pointLightsOn = true, spotLightOn = true;
    bool ambientOn = true, diffuseOn = true, specularOn = true;
};

// ─── Globals ─────────────────────────────────────────────────────────────────
extern Camera        camera;
extern LightingState lighting;
extern bool  dayMode;
extern int   textureMode;  // 0..4  (4 = pillar image texture showcase)
extern float deltaTime;
extern float lastFrame;

// ─── Texture IDs ─────────────────────────────────────────────────────────────
extern unsigned int texBrick, texGrass, texConcrete, texMarble, texWood, texTile;
extern unsigned int texPillar;   // loaded from pillar.png via stb_image

// ─── VAO / VBO handles ───────────────────────────────────────────────────────
extern unsigned int cubeVAO, cubeVBO;
extern unsigned int cylVAO, cylVBO;
extern int cylCount;
extern unsigned int sphVAO, sphVBO;
extern int sphCount;
extern unsigned int conVAO, conVBO;
extern int conCount;

// ─── Colour palette ──────────────────────────────────────────────────────────
extern const glm::vec3 COL_CONCRETE;
extern const glm::vec3 COL_ROOF;
extern const glm::vec3 COL_COLUMN;
extern const glm::vec3 COL_REDBRICK;
extern const glm::vec3 COL_GLASS;
extern const glm::vec3 COL_DOOR;
extern const glm::vec3 COL_PLAZA;
extern const glm::vec3 COL_GRASS;
extern const glm::vec3 COL_TRUNK;
extern const glm::vec3 COL_PALM;
extern const glm::vec3 COL_SIGN;
extern const glm::vec3 COL_BEAM;
extern const glm::vec3 COL_MARBLE;
extern const glm::vec3 COL_STONE;
extern const glm::vec3 COL_LETTER;
extern const glm::vec3 COL_WATER;
extern const glm::vec3 COL_RAILING;
extern const glm::vec3 COL_FLOOR;
extern const glm::vec3 COL_BALCONY;

// ─── Interior detection helper ────────────────────────────────────────────────
// Returns true when the camera is physically inside the left dormitory building
inline bool cameraInsideBuilding() {
    return (camera.position.x > -22.f && camera.position.x < -10.f &&
        camera.position.z > -16.f && camera.position.z < 16.f &&
        camera.position.y < 8.f);
}