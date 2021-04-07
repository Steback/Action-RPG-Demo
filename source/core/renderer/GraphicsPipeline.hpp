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
        GraphicsPipeline(uint shaderID, const vk::Device& device);

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
        uint m_shaderID;
    };

} // namespace vkc


#endif //PROTOTYPE_ACTION_RPG_GRAPHICSPIPELINE_HPP
