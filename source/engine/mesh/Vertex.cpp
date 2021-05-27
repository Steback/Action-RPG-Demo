#include "Vertex.hpp"


namespace engine {

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
                .offset = offsetof(Vertex, normal)
            },
            {
                .location = 2,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = offsetof(Vertex, uv0)
            },
            {
                .location = 3,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = offsetof(Vertex, uv1)
            },
            {
                .location = 4,
                .binding = 0,
                .format = vk::Format::eR32G32B32A32Sfloat,
                .offset = offsetof(Vertex, joint0)
            },
            {
                .location = 5,
                .binding = 0,
                .format = vk::Format::eR32G32B32A32Sfloat,
                .offset = offsetof(Vertex, weight0)
            }
        };
    }

} // namespace core