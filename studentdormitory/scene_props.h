#pragma once
#include <glm/glm.hpp>

void renderPalmTree(unsigned int sh, glm::vec3 base);
void renderConeTree(unsigned int sh, float x, float z);
void renderFractalTree(unsigned int sh, float x, float z);   // NEW: fractal branching tree
void renderBench(unsigned int sh, glm::vec3 pos, float rotY = 0.f);
void renderFountain(unsigned int sh, glm::vec3 pos);
void renderLampPost(unsigned int sh, float x, float z);
void renderGate(unsigned int sh);
void renderTelevision(unsigned int sh);
void renderWalkingStudents(unsigned int sh);   // NEW: animated students in front