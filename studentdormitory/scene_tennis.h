#pragma once

// Tennis court layout constants (shared with scene.cpp for ball shadow Y)
extern const float BC_X;
extern const float BC_Z;
extern const float BC_Y;
extern const float BC_LEN;
extern const float BC_WID;
extern const float BC_SGL;

void renderTennisCourt(unsigned int sh);
void renderTennisPlayers(unsigned int sh);
void renderTennisBall(unsigned int sh);
