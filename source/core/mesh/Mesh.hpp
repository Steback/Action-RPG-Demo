#ifndef PROTOTYPE_ACTION_RPG_MESH_HPP
#define PROTOTYPE_ACTION_RPG_MESH_HPP


#include <array>

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"


struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

class Mesh {

};


#endif //PROTOTYPE_ACTION_RPG_MESH_HPP
