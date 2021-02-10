#include "Utilities.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace core {

   namespace tools {

       std::vector<char> readFile(const std::string& fileName) {
           std::ifstream file(fileName, std::ios::ate | std::ios::binary);

           if (!file.is_open()) core::throw_ex("Failed to open file: " + fileName);

           size_t fileSize = static_cast<size_t>(file.tellg());
           std::vector<char> buffer(fileSize);

           file.seekg(0);
           file.read(buffer.data(), fileSize);

           file.close();

           return buffer;
       }

       stbi_uc* loadTextureFile(const std::string& fileName, int* width, int* height, VkDeviceSize* size) {
           int channels;

           // Load pixel data for image
           std::string fileLoc = TEXTURES_DIR + fileName;
           stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

           if (!image) throw std::runtime_error("Failed to load a Texture file: " + fileName);

           // Calculate image m_size using given and known data
           *size = *width * *height * static_cast<int>(STBI_rgb_alpha);

           return image;
       }

   } // namespace tools

} // namespace core
