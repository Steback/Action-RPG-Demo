#ifndef PROTOTYPE_ACTION_RPG_VERTEX_HPP
#define PROTOTYPE_ACTION_RPG_VERTEX_HPP


#include <array>

#include "glm/glm.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"


namespace core {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texCoord;

        static vk::VertexInputBindingDescription getBindingDescription();

        static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();
    };

} // namesapce core


#endif //PROTOTYPE_ACTION_RPG_VERTEX_HPP
