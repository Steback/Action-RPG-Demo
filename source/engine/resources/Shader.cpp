#include "Shader.hpp"

#include <utility>

#include "../Constants.hpp"
#include "../Utilities.hpp"
#include "../mesh/Vertex.hpp"


inline vk::ShaderModule loadShader(const std::vector<uint32_t> &code, vk::Device device) {
    return device.createShaderModule({
        .codeSize = code.size(),
        .pCode = code.data(),
    });
}

namespace engine {

    Shader::Shader(const std::string &vert, const std::string &frag, vk::Device device, std::vector<vk::PushConstantRange> pushConstants, bool vertexInfo)
            : m_pushConstants(std::move(pushConstants)) {
        m_vertModule = loadShader(engine::tools::readFile(SHADERS_DIR + vert), device);
        m_fragModule = loadShader(engine::tools::readFile(SHADERS_DIR + frag), device);

        if (vertexInfo) {
            m_binding = engine::Vertex::getBindingDescription();
            m_attributes = engine::Vertex::getAttributeDescriptions();
        }
    }

    void Shader::cleanup(const vk::Device& device) {
        device.destroy(m_vertModule);
        device.destroy(m_fragModule);
    }

    void Shader::setAtrributes(const vk::VertexInputBindingDescription &binding, const std::vector<vk::VertexInputAttributeDescription> &attributes) {
        m_binding = binding;
        m_attributes = attributes;
    }

    std::vector<vk::PipelineShaderStageCreateInfo> Shader::getShaderstages() {
        return {
            {
                .stage = vk::ShaderStageFlagBits::eVertex,
                .module = m_vertModule,
                .pName = "main"
            },
            {
                .stage = vk::ShaderStageFlagBits::eFragment,
                .module = m_fragModule,
                .pName = "main"
            }
        };
    }

    vk::VertexInputBindingDescription& Shader::getBinding() {
        return m_binding;
    };

    std::vector<vk::VertexInputAttributeDescription>& Shader::getAttributes() {
        return m_attributes;
    }

    std::vector<vk::PushConstantRange> Shader::getPushConstants() {
        return m_pushConstants;
    }

} // namespace core