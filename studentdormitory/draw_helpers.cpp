#include "draw_helpers.h"
#include "globals.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

void applyTexture(unsigned int shader, unsigned int texID, float uvTile) {
    glUniform1f(glGetUniformLocation(shader, "uvTile"), uvTile);
    int effMode = (texID != 0) ? textureMode : 0;
    glUniform1i(glGetUniformLocation(shader, "texMode"), effMode);
    if (texID != 0 && textureMode != 0) {
        glUniform1i(glGetUniformLocation(shader, "hasTexture"), 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);
        glUniform1i(glGetUniformLocation(shader, "texSampler"), 0);
    }
    else {
        glUniform1i(glGetUniformLocation(shader, "hasTexture"), 0);
    }
}

void drawCube(unsigned int sh, glm::mat4 m, glm::vec3 col, unsigned int tex, float tile) {
    glUniformMatrix4fv(glGetUniformLocation(sh, "model"), 1, GL_FALSE, glm::value_ptr(m));
    glUniform3fv(glGetUniformLocation(sh, "objectColor"), 1, glm::value_ptr(col));
    applyTexture(sh, tex, tile); glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES, 0, 36);
}

void drawGlass(unsigned int sh, glm::mat4 m) {
    static const glm::vec3 gc(0.15f, 0.50f, 1.00f);
    static const glm::vec3 em(0.08f, 0.25f, 0.55f);
    static const glm::vec3 zero(0.f, 0.f, 0.f);
    glUniformMatrix4fv(glGetUniformLocation(sh, "model"), 1, GL_FALSE, glm::value_ptr(m));
    glUniform3fv(glGetUniformLocation(sh, "objectColor"), 1, glm::value_ptr(gc));
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(em));
    glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES, 0, 36);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(zero));
}

void drawCrystalGlass(unsigned int sh, glm::mat4 m) {
    static const glm::vec3 gc(0.85f, 0.95f, 1.00f);
    static const glm::vec3 em(0.40f, 0.70f, 0.90f);
    static const glm::vec3 zero(0.f, 0.f, 0.f);
    glUniformMatrix4fv(glGetUniformLocation(sh, "model"), 1, GL_FALSE, glm::value_ptr(m));
    glUniform3fv(glGetUniformLocation(sh, "objectColor"), 1, glm::value_ptr(gc));
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(em));
    glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES, 0, 36);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(zero));
}

void drawSignCube(unsigned int sh, glm::mat4 m) {
    static const glm::vec3 wc(1.f, 1.f, 1.f);
    static const glm::vec3 em(0.9f, 0.9f, 0.9f);
    static const glm::vec3 zero(0.f, 0.f, 0.f);
    glUniformMatrix4fv(glGetUniformLocation(sh, "model"), 1, GL_FALSE, glm::value_ptr(m));
    glUniform3fv(glGetUniformLocation(sh, "objectColor"), 1, glm::value_ptr(wc));
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(em));
    glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES, 0, 36);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(zero));
}

void drawCylinder(unsigned int sh, glm::mat4 m, glm::vec3 col, unsigned int tex, float tile) {
    glUniformMatrix4fv(glGetUniformLocation(sh, "model"), 1, GL_FALSE, glm::value_ptr(m));
    glUniform3fv(glGetUniformLocation(sh, "objectColor"), 1, glm::value_ptr(col));
    applyTexture(sh, tex, tile); glBindVertexArray(cylVAO); glDrawArrays(GL_TRIANGLES, 0, cylCount);
}

void drawSphere(unsigned int sh, glm::mat4 m, glm::vec3 col, unsigned int tex, float tile) {
    glUniformMatrix4fv(glGetUniformLocation(sh, "model"), 1, GL_FALSE, glm::value_ptr(m));
    glUniform3fv(glGetUniformLocation(sh, "objectColor"), 1, glm::value_ptr(col));
    applyTexture(sh, tex, tile); glBindVertexArray(sphVAO); glDrawArrays(GL_TRIANGLES, 0, sphCount);
}

void drawCone(unsigned int sh, glm::mat4 m, glm::vec3 col, unsigned int tex, float tile) {
    glUniformMatrix4fv(glGetUniformLocation(sh, "model"), 1, GL_FALSE, glm::value_ptr(m));
    glUniform3fv(glGetUniformLocation(sh, "objectColor"), 1, glm::value_ptr(col));
    applyTexture(sh, tex, tile); glBindVertexArray(conVAO); glDrawArrays(GL_TRIANGLES, 0, conCount);
}
