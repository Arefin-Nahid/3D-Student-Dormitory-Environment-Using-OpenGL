#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// ─── Screen dimensions ───────────────────────────────────────────────────────
extern int SCR_WIDTH;
extern int SCR_HEIGHT;
extern const float PI;
extern int activeViewport;

// ─── Room view mode ──────────────────────────────────────────────────────────
extern bool roomViewMode;   // true = O key pressed, camera inside dorm room

// ─── Camera ──────────────────────────────────────────────────────────────────
struct Camera {
    glm::vec3 position{ 0.f, 28.f, 75.f };
    glm::vec3 front{ 0.f, -0.32f, -1.f };
    glm::vec3 up{ 0.f,  1.f,   0.f };
    glm::vec3 right{ 1.f,  0.f,   0.f };
    glm::vec3 worldUp{ 0.f,  1.f,   0.f };
    float yaw = -90.f, pitch = -18.f, roll = 0.f;
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
extern int   textureMode;
extern float deltaTime;
extern float lastFrame;

// ─── Texture IDs ─────────────────────────────────────────────────────────────
extern unsigned int texBrick, texGrass, texConcrete, texMarble, texWood, texTile;

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
