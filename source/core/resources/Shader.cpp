#include "Shader.hpp"

#include "../Utilities.hpp"
#include "../mesh/Vertex.hpp"


inline vk::ShaderModule loadShader(const std::vector<char> &code, vk::Device device) {
    return device.createShaderModule({
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    });
}

namespace core {

    Shader::Shader(const std::string &vert, const std::string &frag,vk::Device device, bool vertexInfo) {
        m_vertModule = loadShader(core::tools::readFile(vert), device);
        m_fragModule = loadShader(core::tools::readFile(frag), device);

        if (vertexInfo) {
            m_binding = core::Vertex::getBindingDescription();
            m_attributes = core::Vertex::getAttributeDescriptions();
        }
    }

    void Shader::cleanup(const vk::Device& device) {
        device.destroy(m_vertModule);
        device.destroy(m_fragModule);
    }

    void Shader::setupAtrributes(const vk::VertexInputBindingDescription &binding, const std::vector<vk::VertexInputAttributeDescription> &attributes) {
        m_binding = binding;
        m_attributes = attributes;
    }

    vk::ShaderModule Shader::getVertex() {
        return m_vertModule;
    }

    vk::ShaderModule Shader::getFragment() {
        return m_fragModule;
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

} // namespace core