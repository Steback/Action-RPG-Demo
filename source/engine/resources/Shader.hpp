#ifndef PROTOTYPE_ACTION_RPG_SHADER_HPP
#define PROTOTYPE_ACTION_RPG_SHADER_HPP


#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"


namespace engine {

    class Shader {
    public:
        explicit Shader(const std::string& vert, const std::string& frag, vk::Device device, std::vector<vk::PushConstantRange> pushConstants, bool vertexInfo = true);

        void cleanup(const vk::Device& device);

        void setAtrributes(const vk::VertexInputBindingDescription &binding, const std::vector<vk::VertexInputAttributeDescription> &attributes);

        std::vector<vk::PipelineShaderStageCreateInfo> getShaderstages();

        vk::VertexInputBindingDescription& getBinding();

        std::vector<vk::VertexInputAttributeDescription>& getAttributes();

        std::vector<vk::PushConstantRange> getPushConstants();

    private:
        vk::ShaderModule m_vertModule;
        vk::ShaderModule m_fragModule;
        vk::VertexInputBindingDescription m_binding;
        std::vector<vk::VertexInputAttributeDescription> m_attributes;
        std::vector<vk::PushConstantRange> m_pushConstants;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_SHADER_HPP
