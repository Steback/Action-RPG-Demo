#ifndef PROTOTYPE_ACTION_RPG_GRAPHICSPIPELINE_HPP
#define PROTOTYPE_ACTION_RPG_GRAPHICSPIPELINE_HPP


#include <string>
#include <array>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"

#include "../resources/Shader.hpp"


namespace vkc {

    class SwapChain;

    class GraphicsPipeline {
    public:
        GraphicsPipeline(std::shared_ptr<core::Shader> shader, const vk::Device& device);

        void create(const std::vector<vk::PushConstantRange>& pushConstants, const std::vector<vk::DescriptorSetLayout>& layouts,
                    const vkc::SwapChain& swapChain, const vk::RenderPass& renderPass, vk::SampleCountFlagBits sampleCount);

        void cleanup();

        void bind(const vk::CommandBuffer& cmdBuffer);

        vk::Pipeline getPipeline();

        vk::PipelineLayout getLayout();

    private:
        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_layout;
        vk::Device m_device;
        std::string m_vertShader;
        std::string m_fragShader;
        std::shared_ptr<core::Shader> m_shader;
    };

} // namespace vkc


#endif //PROTOTYPE_ACTION_RPG_GRAPHICSPIPELINE_HPP
