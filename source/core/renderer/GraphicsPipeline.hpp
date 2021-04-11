#ifndef PROTOTYPE_ACTION_RPG_GRAPHICSPIPELINE_HPP
#define PROTOTYPE_ACTION_RPG_GRAPHICSPIPELINE_HPP


#include <string>
#include <array>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"


namespace core {

    class Shader;
    class SwapChain;

    class GraphicsPipeline {
    public:
        GraphicsPipeline(std::shared_ptr<Shader> shader, const vk::Device& device);

        void create(const std::vector<vk::DescriptorSetLayout>& layouts, const core::SwapChain& swapChain,
                    const vk::RenderPass& renderPass, vk::SampleCountFlagBits sampleCount);

        void cleanup();

        void bind(const vk::CommandBuffer& cmdBuffer);

        vk::Pipeline getPipeline();

        vk::PipelineLayout getLayout();

    private:
        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_layout;
        vk::Device m_device;
        std::shared_ptr<Shader> m_shader;
    };

} // namespace vkc


#endif //PROTOTYPE_ACTION_RPG_GRAPHICSPIPELINE_HPP
