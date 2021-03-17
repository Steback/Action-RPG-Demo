#ifndef PROTOTYPE_ACTION_RPG_CONSTANTS_HPP
#define PROTOTYPE_ACTION_RPG_CONSTANTS_HPP


#include <string>

#include <glm/glm.hpp>


#ifdef NDEBUG
#define CORE_RELEASE
#else
#define CORE_DEBUG
#endif

const int MAX_FRAMES_IN_FLIGHT = 1;
const int MAX_OBJECTS = 100;

const std::string TEXTURES_DIR = "../assets/textures/";
const std::string SHADERS_DIR = "../assets/shaders/";
const std::string FONTS_DIR = "../assets/fonts/";
const std::string MODELS_DIR = "../assets/models/";

const glm::vec3 DEFAULT_SIZE = {0.1f, 0.1f, 0.1f};
const float SPEED_ZERO = 0.0f;
const glm::vec3 DEFAULT_ROTATION = {0.0f, 0.0f, 0.0f};

#endif //PROTOTYPE_ACTION_RPG_CONSTANTS_HPP
