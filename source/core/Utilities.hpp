#ifndef PROTOTYPE_ACTION_RPG_UTILITIES_HPP
#define PROTOTYPE_ACTION_RPG_UTILITIES_HPP

#include <string>
#include <stdexcept>
#include <vector>
#include <fstream>

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include "stb_image.h"

#include "Constants.hpp"


namespace core {

    struct MVP {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    inline void throw_ex(const std::string& message) {
        throw std::runtime_error(message);
    };

    namespace tools {

        std::vector<char> readFile(const std::string& fileName);

        stbi_uc* loadTextureFile(const std::string& fileName, int* width, int* height, VkDeviceSize* size);

    } // namespace tools

} // namespace core

#endif //PROTOTYPE_ACTION_RPG_UTILITIES_HPP
