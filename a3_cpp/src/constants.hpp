#pragma once

#include <glm/glm.hpp>

constexpr glm::vec3 SKY = glm::vec3(0.2f, 0.f, 0.f);
constexpr float PI = 3.141592;
constexpr float EPS = 1e-6f;
constexpr float RAY_EPS = 1e-3f;
constexpr float GAMMA = 2.2f;
constexpr int RAY_TRACING_DEPTH = 6;
constexpr float CAMERA_DELTA_ANGLE_X = 2.0 * PI / 800.0;
constexpr float CAMERA_DELTA_ANGLE_Y = PI / 600.0;

constexpr int WIN_WIDTH = 640;
constexpr int WIN_HEIGHT = 480;
