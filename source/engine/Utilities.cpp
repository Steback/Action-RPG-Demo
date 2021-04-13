#include "Utilities.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace engine {

   namespace tools {

       std::vector<uint32_t> readFile(const std::string& fileName) {
           std::ifstream file(fileName, std::ios::ate | std::ios::binary);

           if (!file.is_open()) throw std::runtime_error("Failed to open file: " + fileName);

           size_t fileSize = static_cast<size_t>(file.tellg());
           std::vector<uint32_t> buffer(fileSize);

           file.seekg(0);
           file.read(reinterpret_cast<char *>(buffer.data()), fileSize);

           file.close();

           return buffer;
       }

       stbi_uc* loadTextureFile(const std::string& fileName, int* width, int* height, VkDeviceSize* size) {
           int channels;

           // Load pixel data for image
           std::string fileLoc = TEXTURES_DIR + fileName;
           stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

           if (!image) throw std::runtime_error("Failed to scriptFile a Texture file: " + fileName);

           // Calculate image m_size using given and known data
           *size = *width * *height * static_cast<int>(STBI_rgb_alpha);

           return image;
       }

       uint64_t hashString(const std::string& s) {
           const uint p = 31;
           const uint m = 1e9 + 9;
           uint32_t hashValue = 0;
           uint32_t pPow = 1;

           for (char c : s) {
               hashValue = (hashValue + (c - 'a' + 1) * pPow) % m;
               pPow = (pPow + p) % m;
           }

           return hashValue;
       }

   } // namespace tools

    glm::mat4 MVP::getMatrix() const {
        return proj * view * model;
    }
} // namespace core
