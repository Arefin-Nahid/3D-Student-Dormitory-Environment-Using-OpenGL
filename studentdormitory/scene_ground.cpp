#include "scene_ground.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

void renderGround(unsigned int sh) {
    // Grass everywhere
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f,-0.05f,0.f }), { 400.f,0.1f,400.f });
    drawCube(sh, m, COL_GRASS, texGrass, 40.f);

    // Front plaza (gate z=28 down to buildings z=16)
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f,0.02f,22.f }), { 56.f,0.08f,12.f });
    drawCube(sh, m, COL_PLAZA, texTile, 6.f);

    // Courtyard central path
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f,0.03f,0.f }), { 4.2f,0.07f,34.f });
    drawCube(sh, m, COL_PLAZA, texTile, 5.f);

    // Left building forecourt
    m = glm::scale(glm::translate(glm::mat4(1.f), { -16.f,0.02f,22.f }), { 12.f,0.07f,12.f });
    drawCube(sh, m, COL_PLAZA, texTile, 3.f);

    // Right building forecourt
    m = glm::scale(glm::translate(glm::mat4(1.f), { 16.f,0.02f,22.f }), { 12.f,0.07f,12.f });
    drawCube(sh, m, COL_PLAZA, texTile, 3.f);
}

void renderBoundaryWall(unsigned int sh) {
    glm::mat4 m;
    // Gate is at z=28, centred x=0, pillar half-gap = 4.80+0.90=5.70 each side → total opening 11.4
    // Boundary must start from the edge of the gate opening
    const float GATE_HALF = 5.70f + 0.90f;  // outer edge of cylinder pillars ≈ 6.6 from centre
    const float WALL_TOTAL_X = 28.f;        // walls extend to x=±28 (matches side walls)
    const float WH = 2.5f, WT = 0.5f;

    // Front wall LEFT segment (x = -28 to -6.6)
    float leftW = WALL_TOTAL_X - GATE_HALF;
    float leftCX = -GATE_HALF - leftW * 0.5f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { leftCX, WH * 0.5f, 28.f }), { leftW, WH, WT });
    drawCube(sh, m, COL_REDBRICK, texBrick, 4.f);

    // Front wall RIGHT segment (x = +6.6 to +28)
    float rightCX = GATE_HALF + leftW * 0.5f;
    m = glm::scale(glm::translate(glm::mat4(1.f), { rightCX, WH * 0.5f, 28.f }), { leftW, WH, WT });
    drawCube(sh, m, COL_REDBRICK, texBrick, 4.f);

    // Left side wall (x=-28, z=-22..+28)
    m = glm::scale(glm::translate(glm::mat4(1.f), { -WALL_TOTAL_X, WH * 0.5f, 3.f }), { WT, WH, 50.f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 8.f);

    // Right side wall (x=+28)
    m = glm::scale(glm::translate(glm::mat4(1.f), { WALL_TOTAL_X, WH * 0.5f, 3.f }), { WT, WH, 50.f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 8.f);

    // Back wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f, WH * 0.5f, -22.f }), { WALL_TOTAL_X * 2.f, WH, WT });
    drawCube(sh, m, COL_REDBRICK, texBrick, 8.f);
}

void renderPaths(unsigned int sh) {
    const glm::vec3 TILE(0.82f, 0.82f, 0.80f);
    const glm::vec3 GROUT(0.60f, 0.60f, 0.58f);
    const float TILE_H = 0.018f, TILE_Y = 0.025f;
    const float TILE_SZ = 1.20f, GAP = 0.08f, STRIDE = TILE_SZ + GAP;

    auto layTiles = [&](float cx, float cz, int nX, int nZ) {
        float ox = -(nX - 1) * 0.5f * STRIDE, oz = -(nZ - 1) * 0.5f * STRIDE;
        for (int iz = 0; iz < nZ; iz++) for (int ix = 0; ix < nX; ix++) {
            float tx = cx + ox + ix * STRIDE, tz = cz + oz + iz * STRIDE;
            glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { tx,TILE_Y,tz }), { TILE_SZ,TILE_H,TILE_SZ });
            drawCube(sh, m, TILE, texTile, 1.f);
            m = glm::scale(glm::translate(glm::mat4(1.f), { tx,TILE_Y - TILE_H * 0.6f,tz }), { TILE_SZ + GAP,TILE_H * 0.5f,TILE_SZ + GAP });
            drawCube(sh, m, GROUT);
        }
        };

    layTiles(0.f, 22.5f, 3, 8);   // main entrance path
    layTiles(-8.5f, 0.f, 2, 26);  // left side path
    layTiles(8.5f, 0.f, 2, 26);  // right side path

    // Entrance steps for each building
    for (int step = 0; step < 3; step++) {
        float sy = TILE_Y + step * 0.10f;
        glm::mat4 m;
        m = glm::scale(glm::translate(glm::mat4(1.f), { -16.f,sy,16.3f - step * 0.55f }), { 8.f,TILE_H + 0.08f,0.55f });
        drawCube(sh, m, glm::vec3(0.80f + step * 0.03f, 0.78f + step * 0.02f, 0.74f + step * 0.02f), texTile, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { 16.f,sy,16.3f - step * 0.55f }), { 8.f,TILE_H + 0.08f,0.55f });
        drawCube(sh, m, glm::vec3(0.80f + step * 0.03f, 0.78f + step * 0.02f, 0.74f + step * 0.02f), texTile, 2.f);
    }

    const glm::vec3 KERB(0.55f, 0.55f, 0.53f);
    glm::mat4 m;
    m = glm::scale(glm::translate(glm::mat4(1.f), { -1.8f,TILE_Y + 0.06f,0.f }), { 0.20f,0.12f,34.f }); drawCube(sh, m, KERB, texConcrete, 5.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { 1.8f,TILE_Y + 0.06f,0.f }), { 0.20f,0.12f,34.f }); drawCube(sh, m, KERB, texConcrete, 5.f);
}
