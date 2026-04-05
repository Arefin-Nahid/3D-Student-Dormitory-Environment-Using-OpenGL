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
