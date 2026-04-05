#include "textures.h"
#include "globals.h"
#include <glad/glad.h>
#include <cmath>
#include <algorithm>

unsigned int uploadTexture(unsigned char* data, int w, int h) {
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}

unsigned char clamp8(int v) {
    return (unsigned char)std::max(0, std::min(255, v));
}

unsigned int createBrickTexture() {
    const int W = 128, H = 64;
    static unsigned char data[W * H * 3];
    const int bH = 14, bW = 32, mort = 2;
    for (int y = 0; y < H; y++) {
        int row = y / bH, off = (row % 2) * (bW / 2);
        bool yM = (y % bH) < mort;
        for (int x = 0; x < W; x++) {
            bool xM = ((x + off) % bW) < mort;
            int i = (y * W + x) * 3;
            if (yM || xM) { data[i] = 148; data[i + 1] = 138; data[i + 2] = 128; }
            else {
                int var = ((x * 7 + y * 13 + row * 31) % 40) - 20;
                data[i] = clamp8(185 + var); data[i + 1] = clamp8(72 + var / 2); data[i + 2] = clamp8(48 + var / 3);
            }
        }
    }
    return uploadTexture(data, W, H);
}

unsigned int createGrassTexture() {
    const int W = 64, H = 64;
    static unsigned char data[W * H * 3];
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            int i = (y * W + x) * 3;
            float n = sinf(x * 0.7f) * cosf(y * 0.5f) + sinf(x * 1.3f + y * 0.9f) * 0.5f;
            int v = (int)(n * 18);
            data[i] = clamp8(50 + v); data[i + 1] = clamp8(132 + v * 2); data[i + 2] = clamp8(40 + v);
        }
    return uploadTexture(data, W, H);
}

unsigned int createConcreteTexture() {
    const int W = 64, H = 64;
    static unsigned char data[W * H * 3];
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            int i = (y * W + x) * 3;
            float n = sinf(x * 2.1f + y * 3.7f) * cosf(x * 1.3f - y * 2.9f);
            int v = (int)(n * 10);
            data[i] = clamp8(198 + v); data[i + 1] = clamp8(196 + v); data[i + 2] = clamp8(192 + v);
        }
    return uploadTexture(data, W, H);
}

unsigned int createMarbleTexture() {
    const int W = 128, H = 128;
    static unsigned char data[W * H * 3];
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            int i = (y * W + x) * 3;
            float t = sinf((x + y * 0.5f) * 0.15f + sinf(x * 0.08f) * 2.0f + sinf(y * 0.12f) * 1.5f);
            t = (t + 1.0f) * 0.5f;
            int v = (int)(195 + t * 55);
            data[i] = clamp8(v + 6); data[i + 1] = clamp8(v + 2); data[i + 2] = clamp8(v - 4);
        }
    return uploadTexture(data, W, H);
}

unsigned int createWoodTexture() {
    const int W = 64, H = 64;
    static unsigned char data[W * H * 3];
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            int i = (y * W + x) * 3;
            float r = sqrtf((float)((x - 32) * (x - 32) + (y - 32) * (y - 32)));
            float ring = (sinf(r * 0.8f + sinf(x * 0.3f) * 1.5f) + 1.0f) * 0.5f;
            data[i] = clamp8(100 + (int)(ring * 80));
            data[i + 1] = clamp8(60 + (int)(ring * 50));
            data[i + 2] = clamp8(30 + (int)(ring * 20));
        }
    return uploadTexture(data, W, H);
}

unsigned int createTileTexture() {
    const int W = 64, H = 64;
    static unsigned char data[W * H * 3];
    const int tileSize = 16, grout = 1;
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            int i = (y * W + x) * 3;
            bool gx = (x % tileSize) < grout;
            bool gy = (y % tileSize) < grout;
            if (gx || gy) { data[i] = 160; data[i + 1] = 158; data[i + 2] = 155; }
            else {
                int var = ((x * 5 + y * 11) % 20) - 10;
                data[i] = clamp8(215 + var); data[i + 1] = clamp8(210 + var); data[i + 2] = clamp8(205 + var);
            }
        }
    return uploadTexture(data, W, H);
}

void createAllTextures() {
    texBrick    = createBrickTexture();
    texGrass    = createGrassTexture();
    texConcrete = createConcreteTexture();
    texMarble   = createMarbleTexture();
    texWood     = createWoodTexture();
    texTile     = createTileTexture();
}
