#ifndef PROTOTYPE_ACTION_RPG_UTILITIES_HPP
#define PROTOTYPE_ACTION_RPG_UTILITIES_HPP


#include <string>
#include <stdexcept>
#include <vector>
#include <fstream>

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include "stb_image.h"
#include "fmt/format.h"

#include "Constants.hpp"


#define VK_CHECK_RESULT_HPP(f) { \
    vk::Result res = (f); \
    if (res != vk::Result::eSuccess) throw std::runtime_error(fmt::format("VkResult is \"{}\" in {} at line {} \n", vk::to_string(res), __FILE__, __LINE__)); \
}

namespace engine {

    struct MVP {
        alignas(16) glm::mat4 proj;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 model;

        glm::mat4 getMatrix() const;
    };

    inline void throw_ex(const std::string& message) {
        throw std::runtime_error(message);
    };

    namespace tools {

        std::vector<uint32_t> readFile(const std::string& fileName);

        stbi_uc* loadTextureFile(const std::string& fileName, int* width, int* height, VkDeviceSize* size);

        uint64_t hashString(const std::string& s);

    } // namespace tools

} // namespace core

#endif //PROTOTYPE_ACTION_RPG_UTILITIES_HPP
