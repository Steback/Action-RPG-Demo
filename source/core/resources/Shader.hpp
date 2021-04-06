#ifndef PROTOTYPE_ACTION_RPG_SHADER_HPP
#define PROTOTYPE_ACTION_RPG_SHADER_HPP


#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"


namespace core {

    class Shader {
    public:
        explicit Shader(const std::string& vert, const std::string& frag, vk::Device device, bool vertexInfo = true);

        void cleanup(const vk::Device& device);

        void setupAtrributes(const vk::VertexInputBindingDescription &binding, const std::vector<vk::VertexInputAttributeDescription> &attributes);

        vk::ShaderModule getVertex();

        vk::ShaderModule getFragment();

        std::vector<vk::PipelineShaderStageCreateInfo> getShaderstages();

        vk::VertexInputBindingDescription& getBinding();

        std::vector<vk::VertexInputAttributeDescription>& getAttributes();

    private:
        vk::ShaderModule m_vertModule;
        vk::ShaderModule m_fragModule;
        vk::VertexInputBindingDescription m_binding;
        std::vector<vk::VertexInputAttributeDescription> m_attributes;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_SHADER_HPP
