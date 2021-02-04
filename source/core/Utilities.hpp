#ifndef PROTOTYPE_ACTION_RPG_UTILITIES_HPP
#define PROTOTYPE_ACTION_RPG_UTILITIES_HPP


#include <string>
#include <stdexcept>
#include <vector>
#include <fstream>

#include "glm/glm.hpp"
#include "stb_image.h"

#include "Constants.hpp"

#ifdef NDEBUG
#define CORE_RELEASE
#else
#define CORE_DEBUG
#endif


namespace core {

    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    inline void throw_ex(const std::string& message) {
        throw std::runtime_error(message);
    };

    namespace tools {

        inline std::vector<char> readFile(const std::string& fileName) {
            std::ifstream file(fileName, std::ios::ate | std::ios::binary);

            if (!file.is_open()) core::throw_ex("Failed to open file: " + fileName);

            size_t fileSize = static_cast<size_t>(file.tellg());
            std::vector<char> buffer(fileSize);

            file.seekg(0);
            file.read(buffer.data(), fileSize);

            file.close();

            return buffer;
        }

        inline stbi_uc* loadTextureFile(const std::string& fileName, int* width, int* height, VkDeviceSize* size) {
            int channels;

            // Load pixel data for image
            std::string fileLoc = TEXTURES_DIR + fileName;
            stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

            if (!image) throw std::runtime_error("Failed to load a Texture file: " + fileName);

            // Calculate image size using given and known data
            *size = *width * *height * static_cast<int>(STBI_rgb_alpha);

            return image;
        }

    } // namespace tools

} // namespace core

#endif //PROTOTYPE_ACTION_RPG_UTILITIES_HPP
