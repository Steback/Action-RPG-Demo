#include "Vertex.hpp"


namespace core {

    vk::VertexInputBindingDescription Vertex::getBindingDescription() {
        return {
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = vk::VertexInputRate::eVertex
        };
    }

    std::vector<vk::VertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
        return {
            {
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = offsetof(Vertex, position)
            },
            {
                .location = 1,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = offsetof(Vertex, color)
            },
            {
                .location = 2,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = offsetof(Vertex, texCoord)
            }
        };
    }

} // namespace core