#include "scene.h"
#include "globals.h"
#include "draw_helpers.h"
#include "scene_ground.h"
#include "scene_buildings.h"
#include "scene_props.h"
#include "scene_tennis.h"
#include "scene_football.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void renderScene(unsigned int sh) {
    if (roomViewMode) {
        renderDormRoom(sh);
        return;
    }

    // Ground & boundaries
    renderGround(sh);
    renderBoundaryWall(sh);
    renderPaths(sh);

    // Main buildings
    renderDormBlockA(sh);   // Left  – student living
    renderDormBlockB(sh);   // Right – common facilities
    renderCorridor(sh);     // Covered walkways
    renderCourtyard(sh);    // Central courtyard (grass, staircase, benches, trees)
    renderAdminAnnex(sh);   // Back utility block
    //renderEntranceSphere(sh); // Gate pillar finials

    // Gate
    renderGate(sh);

    // Courtyard lamp posts along central path
    renderLampPost(sh, -3.5f, 13.0f);
    renderLampPost(sh, 3.5f, 13.0f);
    renderLampPost(sh, -3.5f, 0.0f);
    renderLampPost(sh, 3.5f, 0.0f);
    renderLampPost(sh, -3.5f, -10.0f);
    renderLampPost(sh, 3.5f, -10.0f);

    // Palm trees at building corners
    renderPalmTree(sh, { -26.f, 0.f,  18.f });
    renderPalmTree(sh, { -26.f, 0.f,  -2.f });
    renderPalmTree(sh, { 26.f, 0.f,  18.f });
    renderPalmTree(sh, { 26.f, 0.f,  -2.f });

    // Cone trees outside boundary
    renderConeTree(sh, -26.f, -14.f);
    renderConeTree(sh, 26.f, -14.f);

    // TWO FRACTAL TREES flanking the gate entrance
    renderFractalTree(sh, -12.f, 24.f);
    renderFractalTree(sh, 12.f, 24.f);

    // Walking students in front of dormitory
    renderWalkingStudents(sh);

    // Sports facilities
    renderTennisCourt(sh);
    renderTennisPlayers(sh);
    renderTennisBall(sh);
    renderFootballField(sh);
    renderFootballPlayers(sh);
    renderFootball(sh);
}