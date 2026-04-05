#include "globals.h"

int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;
const float PI = 3.14159265f;
int activeViewport = -1;
bool roomViewMode = false;

Camera        camera;
LightingState lighting;
bool  dayMode = true;
int   textureMode = 0;
float deltaTime = 0.f;
float lastFrame = 0.f;

unsigned int texBrick, texGrass, texConcrete, texMarble, texWood, texTile;

unsigned int cubeVAO, cubeVBO;
unsigned int cylVAO, cylVBO;  int cylCount;
unsigned int sphVAO, sphVBO;  int sphCount;
unsigned int conVAO, conVBO;  int conCount;

// ─── Colour palette ──────────────────────────────────────────────────────────
const glm::vec3 COL_CONCRETE(0.82f, 0.82f, 0.80f);
const glm::vec3 COL_ROOF(0.70f, 0.72f, 0.75f);
const glm::vec3 COL_COLUMN(0.92f, 0.91f, 0.89f);
const glm::vec3 COL_REDBRICK(0.72f, 0.28f, 0.18f);
const glm::vec3 COL_GLASS(0.20f, 0.55f, 1.00f);
const glm::vec3 COL_DOOR(0.25f, 0.18f, 0.12f);
const glm::vec3 COL_PLAZA(0.78f, 0.72f, 0.65f);
const glm::vec3 COL_GRASS(0.25f, 0.52f, 0.22f);
const glm::vec3 COL_TRUNK(0.40f, 0.28f, 0.15f);
const glm::vec3 COL_PALM(0.15f, 0.55f, 0.20f);
const glm::vec3 COL_SIGN(0.95f, 0.95f, 0.95f);
const glm::vec3 COL_BEAM(0.68f, 0.68f, 0.66f);
const glm::vec3 COL_MARBLE(0.92f, 0.90f, 0.87f);
const glm::vec3 COL_STONE(0.55f, 0.50f, 0.48f);
const glm::vec3 COL_LETTER(0.15f, 0.08f, 0.05f);
const glm::vec3 COL_WATER(0.18f, 0.45f, 0.75f);
const glm::vec3 COL_RAILING(0.60f, 0.60f, 0.62f);
const glm::vec3 COL_FLOOR(0.80f, 0.78f, 0.74f);
const glm::vec3 COL_BALCONY(0.85f, 0.84f, 0.82f);