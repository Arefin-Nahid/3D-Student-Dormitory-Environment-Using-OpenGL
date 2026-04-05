#pragma once
#include <glm/glm.hpp>
#include <string>
#include <GLFW/glfw3.h>

void updateCameraVectors();
glm::mat4 customPerspective(float fovY, float aspect, float zN, float zF);
void framebuffer_size_callback(GLFWwindow* window, int w, int h);
std::string itos(int v);
void setupLighting(unsigned int sh);
