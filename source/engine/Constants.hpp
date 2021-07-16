#ifndef PROTOTYPE_ACTION_RPG_CONSTANTS_HPP
#define PROTOTYPE_ACTION_RPG_CONSTANTS_HPP


#include <string>

#include "glm/glm.hpp"


#ifdef NDEBUG
#define CORE_RELEASE
#else
#define CORE_DEBUG
#endif

const int MAX_FRAMES_IN_FLIGHT = 1;
const int MAX_OBJECTS = 100;

const std::string TEXTURES_DIR = "..\\..\\Assets\\textures\\";
const std::string SHADERS_DIR = "..\\shaders\\";
const std::string FONTS_DIR = "..\\..\\Assets\\fonts\\";
const std::string MODELS_DIR = "..\\..\\Assets\\models\\";
const std::string ANIMATIONS_DIR = "..\\..\\Assets\\animations\\";
const std::string SCRIPTS_DIR = "..\\..\\scripts\\";

const glm::vec3 DEFAULT_SIZE = {1.0f, 1.0f, 1.0f};
const float SPEED_ZERO = 0.0f;
const glm::vec3 DEFAULT_ROTATION = {0.0f, 0.0f, 0.0f};

const glm::vec3 YUP = {0.0f, 1.0f, 0.0f};
const float Z_NEAR_PLANE = 0.01f;
const float Z_FAR_PLANE = 100.00f;
const float FOV = 45.0f;

#endif //PROTOTYPE_ACTION_RPG_CONSTANTS_HPP
