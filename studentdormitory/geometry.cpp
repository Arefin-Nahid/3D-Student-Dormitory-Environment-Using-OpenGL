#include "geometry.h"
#include "globals.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

void setupCube() {
    float v[] = {
        -0.5f,-0.5f,-0.5f, 0,0,-1, 1,0,  0.5f,-0.5f,-0.5f, 0,0,-1, 0,0,  0.5f, 0.5f,-0.5f, 0,0,-1, 0,1,
         0.5f, 0.5f,-0.5f, 0,0,-1, 0,1, -0.5f, 0.5f,-0.5f, 0,0,-1, 1,1, -0.5f,-0.5f,-0.5f, 0,0,-1, 1,0,
        -0.5f,-0.5f, 0.5f, 0,0,1,  0,0,  0.5f,-0.5f, 0.5f, 0,0,1,  1,0,  0.5f, 0.5f, 0.5f, 0,0,1,  1,1,
         0.5f, 0.5f, 0.5f, 0,0,1,  1,1, -0.5f, 0.5f, 0.5f, 0,0,1,  0,1, -0.5f,-0.5f, 0.5f, 0,0,1,  0,0,
        -0.5f, 0.5f, 0.5f,-1,0,0,  1,1, -0.5f, 0.5f,-0.5f,-1,0,0,  0,1, -0.5f,-0.5f,-0.5f,-1,0,0,  0,0,
        -0.5f,-0.5f,-0.5f,-1,0,0,  0,0, -0.5f,-0.5f, 0.5f,-1,0,0,  1,0, -0.5f, 0.5f, 0.5f,-1,0,0,  1,1,
         0.5f, 0.5f, 0.5f, 1,0,0,  0,1,  0.5f, 0.5f,-0.5f, 1,0,0,  1,1,  0.5f,-0.5f,-0.5f, 1,0,0,  1,0,
         0.5f,-0.5f,-0.5f, 1,0,0,  1,0,  0.5f,-0.5f, 0.5f, 1,0,0,  0,0,  0.5f, 0.5f, 0.5f, 1,0,0,  0,1,
        -0.5f,-0.5f,-0.5f, 0,-1,0, 0,0,  0.5f,-0.5f,-0.5f, 0,-1,0, 1,0,  0.5f,-0.5f, 0.5f, 0,-1,0, 1,1,
         0.5f,-0.5f, 0.5f, 0,-1,0, 1,1, -0.5f,-0.5f, 0.5f, 0,-1,0, 0,1, -0.5f,-0.5f,-0.5f, 0,-1,0, 0,0,
        -0.5f, 0.5f,-0.5f, 0,1,0,  0,1,  0.5f, 0.5f,-0.5f, 0,1,0,  1,1,  0.5f, 0.5f, 0.5f, 0,1,0,  1,0,
         0.5f, 0.5f, 0.5f, 0,1,0,  1,0, -0.5f, 0.5f, 0.5f, 0,1,0,  0,0, -0.5f, 0.5f,-0.5f, 0,1,0,  0,1
    };
    glGenVertexArrays(1, &cubeVAO); glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);               glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
}

void setupCylinder(int segs) {
    std::vector<float> v;
    const float r = 0.5f, h = 1.0f;
    for (int i = 0; i < segs; ++i) {
        float t0 = 2.f * PI * (float)i / segs, t1 = 2.f * PI * (float)(i + 1) / segs;
        float u0 = (float)i / segs, u1 = (float)(i + 1) / segs;
        float x0 = r * cosf(t0), z0 = r * sinf(t0), x1 = r * cosf(t1), z1 = r * sinf(t1);
        v.insert(v.end(), { x0,-h / 2,z0, x0,0,z0, u0,0 }); v.insert(v.end(), { x1,-h / 2,z1, x1,0,z1, u1,0 }); v.insert(v.end(), { x0,h / 2,z0, x0,0,z0, u0,1 });
        v.insert(v.end(), { x1,-h / 2,z1, x1,0,z1, u1,0 }); v.insert(v.end(), { x1,h / 2,z1, x1,0,z1, u1,1 }); v.insert(v.end(), { x0,h / 2,z0, x0,0,z0, u0,1 });
        v.insert(v.end(), { 0,h / 2,0, 0,1,0, 0.5f,0.5f }); v.insert(v.end(), { x0,h / 2,z0, 0,1,0, 0.5f + 0.5f * cosf(t0),0.5f + 0.5f * sinf(t0) }); v.insert(v.end(), { x1,h / 2,z1, 0,1,0, 0.5f + 0.5f * cosf(t1),0.5f + 0.5f * sinf(t1) });
        v.insert(v.end(), { 0,-h / 2,0, 0,-1,0, 0.5f,0.5f }); v.insert(v.end(), { x1,-h / 2,z1, 0,-1,0, 0.5f + 0.5f * cosf(t1),0.5f + 0.5f * sinf(t1) }); v.insert(v.end(), { x0,-h / 2,z0, 0,-1,0, 0.5f + 0.5f * cosf(t0),0.5f + 0.5f * sinf(t0) });
    }
    glGenVertexArrays(1, &cylVAO); glGenBuffers(1, &cylVBO);
    glBindVertexArray(cylVAO); glBindBuffer(GL_ARRAY_BUFFER, cylVBO);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);               glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    cylCount = (int)(v.size() / 8);
}

void setupSphere(int stacks, int sectors) {
    std::vector<float> v;
    for (int i = 0; i < stacks; i++) {
        float phi0 = PI * ((float)i / stacks - 0.5f), phi1 = PI * ((float)(i + 1) / stacks - 0.5f);
        for (int j = 0; j < sectors; j++) {
            float th0 = 2.f * PI * (float)j / sectors, th1 = 2.f * PI * (float)(j + 1) / sectors;
            auto vert = [&](float phi, float th) {
                float cp = cosf(phi), sp = sinf(phi), ct = cosf(th), st = sinf(th);
                float nx = cp * ct, ny = sp, nz = cp * st;
                v.insert(v.end(), { nx * 0.5f,ny * 0.5f,nz * 0.5f, nx,ny,nz, th / (2.f * PI),(phi + PI * 0.5f) / PI });
            };
            vert(phi0, th0); vert(phi0, th1); vert(phi1, th0);
            vert(phi0, th1); vert(phi1, th1); vert(phi1, th0);
        }
    }
    glGenVertexArrays(1, &sphVAO); glGenBuffers(1, &sphVBO);
    glBindVertexArray(sphVAO); glBindBuffer(GL_ARRAY_BUFFER, sphVBO);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);               glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    sphCount = (int)(v.size() / 8);
}

void setupCone(int segs) {
    std::vector<float> v;
    const float r = 0.5f, h = 1.0f, slope = r / h;
    for (int i = 0; i < segs; i++) {
        float t0 = 2.f * PI * (float)i / segs, t1 = 2.f * PI * (float)(i + 1) / segs;
        float u0 = (float)i / segs, u1 = (float)(i + 1) / segs;
        float x0 = r * cosf(t0), z0 = r * sinf(t0), x1 = r * cosf(t1), z1 = r * sinf(t1);
        glm::vec3 n0 = glm::normalize(glm::vec3(cosf(t0), slope, sinf(t0)));
        glm::vec3 n1 = glm::normalize(glm::vec3(cosf(t1), slope, sinf(t1)));
        glm::vec3 na = glm::normalize(glm::vec3(cosf((t0 + t1) * 0.5f), slope, sinf((t0 + t1) * 0.5f)));
        v.insert(v.end(), { x0,-h / 2,z0, n0.x,n0.y,n0.z, u0,0 });
        v.insert(v.end(), { x1,-h / 2,z1, n1.x,n1.y,n1.z, u1,0 });
        v.insert(v.end(), { 0,h / 2,0, na.x,na.y,na.z, (u0 + u1) * 0.5f,1 });
        v.insert(v.end(), { 0,-h / 2,0, 0,-1,0, 0.5f,0.5f });
        v.insert(v.end(), { x1,-h / 2,z1, 0,-1,0, 0.5f + 0.5f * cosf(t1),0.5f + 0.5f * sinf(t1) });
        v.insert(v.end(), { x0,-h / 2,z0, 0,-1,0, 0.5f + 0.5f * cosf(t0),0.5f + 0.5f * sinf(t0) });
    }
    glGenVertexArrays(1, &conVAO); glGenBuffers(1, &conVBO);
    glBindVertexArray(conVAO); glBindBuffer(GL_ARRAY_BUFFER, conVBO);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);               glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    conCount = (int)(v.size() / 8);
}
