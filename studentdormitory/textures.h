#pragma once

unsigned int uploadTexture(unsigned char* data, int w, int h);
unsigned char clamp8(int v);
unsigned int createBrickTexture();
unsigned int createGrassTexture();
unsigned int createConcreteTexture();
unsigned int createMarbleTexture();
unsigned int createWoodTexture();
unsigned int createTileTexture();
void createAllTextures();

// Load pillar.png (or any image) via stb_image; returns 0 on failure
unsigned int loadTextureFromFile(const char* path);