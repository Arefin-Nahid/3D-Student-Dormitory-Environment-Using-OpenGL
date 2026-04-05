#include "scene_ground.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

void renderGround(unsigned int sh) {
    // Main grass field surrounding the campus
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f, -0.05f, 0.f }), { 300.f, 0.1f, 300.f });
    drawCube(sh, m, COL_GRASS, texGrass, 30.f);

    // Front paved plaza (between gate and courtyard)
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f, 0.02f, 20.5f }), { 46.f, 0.08f, 16.f });
    drawCube(sh, m, COL_PLAZA, texTile, 6.f);

    // Courtyard central concrete path (x=-2..2, z=-16..+16)
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f, 0.03f, 0.f }), { 4.2f, 0.07f, 34.f });
    drawCube(sh, m, COL_PLAZA, texTile, 5.f);

    // Left building forecourt (x=-22..-10, z=16..28)
    m = glm::scale(glm::translate(glm::mat4(1.f), { -16.f, 0.02f, 22.f }), { 12.f, 0.07f, 12.f });
    drawCube(sh, m, COL_PLAZA, texTile, 3.f);

    // Right building forecourt (x=10..22, z=16..28)
    m = glm::scale(glm::translate(glm::mat4(1.f), { 16.f, 0.02f, 22.f }), { 12.f, 0.07f, 12.f });
    drawCube(sh, m, COL_PLAZA, texTile, 3.f);
}

void renderBoundaryWall(unsigned int sh) {
    glm::mat4 m;
    // Left outer wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { -28.f, 1.25f, 5.f }), { 0.45f, 2.5f, 56.f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 6.f);
    // Right outer wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { 28.f, 1.25f, 5.f }), { 0.45f, 2.5f, 56.f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 6.f);
    // Back wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f, 1.25f, -22.f }), { 58.f, 2.5f, 0.45f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 8.f);
    // Front wall left segment
    m = glm::scale(glm::translate(glm::mat4(1.f), { -17.f, 1.25f, 28.f }), { 22.f, 2.5f, 0.45f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 4.f);
    // Front wall right segment
    m = glm::scale(glm::translate(glm::mat4(1.f), { 17.f, 1.25f, 28.f }), { 22.f, 2.5f, 0.45f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 4.f);
}

void renderPaths(unsigned int sh) {
    const glm::vec3 TILE_COL(0.82f, 0.82f, 0.80f);
    const glm::vec3 GROUT_COL(0.60f, 0.60f, 0.58f);
    const float TILE_H = 0.018f, TILE_Y = 0.025f;
    const float TILE_SZ = 1.20f, GAP = 0.08f, STRIDE = TILE_SZ + GAP;

    auto layTiles = [&](float cx, float cz, int nX, int nZ) {
        float ox = -(nX - 1) * 0.5f * STRIDE, oz = -(nZ - 1) * 0.5f * STRIDE;
        for (int iz = 0; iz < nZ; iz++) for (int ix = 0; ix < nX; ix++) {
            float tx = cx + ox + ix * STRIDE, tz = cz + oz + iz * STRIDE;
            glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { tx, TILE_Y, tz }), { TILE_SZ, TILE_H, TILE_SZ });
            drawCube(sh, m, TILE_COL, texTile, 1.f);
            m = glm::scale(glm::translate(glm::mat4(1.f), { tx, TILE_Y - TILE_H * 0.6f, tz }), { TILE_SZ + GAP, TILE_H * 0.5f, TILE_SZ + GAP });
            drawCube(sh, m, GROUT_COL);
        }
        };

    layTiles(0.f, 22.5f, 3, 8);
    layTiles(-8.5f, 0.f, 2, 26);
    layTiles(8.5f, 0.f, 2, 26);

    for (int step = 0; step < 3; step++) {
        float sy = TILE_Y + step * 0.10f;
        glm::mat4 m;
        m = glm::scale(glm::translate(glm::mat4(1.f), { -16.f, sy, 16.3f - step * 0.55f }),
            { 8.f, TILE_H + 0.08f, 0.55f });
        drawCube(sh, m, glm::vec3(0.80f + step * 0.03f, 0.78f + step * 0.02f, 0.74f + step * 0.02f), texTile, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { 16.f, sy, 16.3f - step * 0.55f }),
            { 8.f, TILE_H + 0.08f, 0.55f });
        drawCube(sh, m, glm::vec3(0.80f + step * 0.03f, 0.78f + step * 0.02f, 0.74f + step * 0.02f), texTile, 2.f);
    }

    const glm::vec3 KERB(0.55f, 0.55f, 0.53f);
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { -1.8f, TILE_Y + 0.06f, 0.f }), { 0.20f, 0.12f, 34.f });
    drawCube(sh, m, KERB, texConcrete, 5.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { 1.8f, TILE_Y + 0.06f, 0.f }), { 0.20f, 0.12f, 34.f });
    drawCube(sh, m, KERB, texConcrete, 5.f);
}
// NOTE: renderEntranceSphere is NOT defined here.
// It is declared in scene_buildings.h and defined in scene_buildings.cpp.