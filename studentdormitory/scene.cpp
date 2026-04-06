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
    // Ground & boundaries
    renderGround(sh);
    renderBoundaryWall(sh);
    renderPaths(sh);

    // Main buildings (interior always rendered; visible when camera enters)
    renderDormBlockA(sh);
    renderDormBlockB(sh);
    renderCorridor(sh);
    renderCourtyard(sh);
    renderAdminAnnex(sh);
    renderEntranceSphere(sh);

    // Gate with cylindrical pillars + texPillar in mode 4
    renderGate(sh);

    // Courtyard lamp posts
    renderLampPost(sh, -3.5f, 13.0f);
    renderLampPost(sh, 3.5f, 13.0f);
    renderLampPost(sh, -3.5f, 0.0f);
    renderLampPost(sh, 3.5f, 0.0f);
    renderLampPost(sh, -3.5f, -10.0f);
    renderLampPost(sh, 3.5f, -10.0f);

    // Palm trees at corners
    renderPalmTree(sh, { -26.f, 0.f,  18.f });
    renderPalmTree(sh, { -26.f, 0.f,  -2.f });
    renderPalmTree(sh, { 26.f, 0.f,  18.f });
    renderPalmTree(sh, { 26.f, 0.f,  -2.f });

    // Cone trees outside boundary
    renderConeTree(sh, -26.f, -14.f);
    renderConeTree(sh, 26.f, -14.f);

    // Fractal trees flanking the gate
    renderFractalTree(sh, -12.f, 24.f);
    renderFractalTree(sh, 12.f, 24.f);

    // Walking students in front of dormitory
    renderWalkingStudents(sh);

    // Sports (tennis LEFT at x=-52, football RIGHT at x=+52)
    renderTennisCourt(sh);
    renderTennisPlayers(sh);
    renderTennisBall(sh);
    renderFootballField(sh);
    renderFootballPlayers(sh);
    renderFootball(sh);
}