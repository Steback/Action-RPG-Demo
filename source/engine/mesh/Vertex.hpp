#ifndef PROTOTYPE_ACTION_RPG_VERTEX_HPP
#define PROTOTYPE_ACTION_RPG_VERTEX_HPP


#include <vector>

#include "glm/glm.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"


namespace engine {

    struct Vertex {
        glm::vec3 position{};
        glm::vec3 normal{};
        glm::vec2 uv0{};
        glm::vec2 uv1{};
        glm::vec4 joint0{};
        glm::vec4 weight0{};

        static vk::VertexInputBindingDescription getBindingDescription();

        static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();
    };

} // namesapce core


#endif //PROTOTYPE_ACTION_RPG_VERTEX_HPP
