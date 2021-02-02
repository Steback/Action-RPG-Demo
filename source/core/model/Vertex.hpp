#ifndef PROTOTYPE_ACTION_RPG_VERTEX_HPP
#define PROTOTYPE_ACTION_RPG_VERTEX_HPP


#include <array>

#include "glm/glm.hpp"
#include "vulkan/vulkan.h"


namespace core {

    struct Vertex {
        glm::vec2 position;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription();

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    };

} // namesapce core


#endif //PROTOTYPE_ACTION_RPG_VERTEX_HPP
