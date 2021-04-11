#include "GraphicsPipeline.hpp"

#include <utility>

#include "SwapChain.hpp"
#include "../Utilities.hpp"
#include "../Application.hpp"
#include "../resources/Shader.hpp"


namespace engine {

    GraphicsPipeline::GraphicsPipeline(std::shared_ptr<Shader> shader, const vk::Device& device)
            : m_shader(std::move(shader)), m_device(device) {

    }

    void GraphicsPipeline::create(const std::vector<vk::DescriptorSetLayout>& layouts, const engine::SwapChain& swapChain,
                                  const vk::RenderPass& renderPass, vk::SampleCountFlagBits sampleCount) {
        auto attributes = m_shader->getAttributes();

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &m_shader->getBinding(),
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size()),
                .pVertexAttributeDescriptions = attributes.data()
        };

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
                .topology = vk::PrimitiveTopology::eTriangleList,
                .primitiveRestartEnable = VK_FALSE
        };

        vk::Viewport viewport{
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(swapChain.getExtent().width),
                .height = static_cast<float>(swapChain.getExtent().height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f
        };

        vk::Rect2D scissor{
                .offset = {0, 0},
                .extent = swapChain.getExtent()
        };

        vk::PipelineViewportStateCreateInfo viewportState{
                .viewportCount = 1,
                .pViewports = &viewport,
                .scissorCount = 1,
                .pScissors = &scissor,
        };

        vk::PipelineRasterizationStateCreateInfo rasterizer{
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = vk::PolygonMode::eFill,
                .cullMode = vk::CullModeFlagBits::eBack,
                .frontFace = vk::FrontFace::eCounterClockwise,
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0.0f,
                .depthBiasClamp = 0.0f,
                .depthBiasSlopeFactor = 0.0f,
                .lineWidth = 1.0f
        };

        vk::PipelineMultisampleStateCreateInfo multisampling{
                .rasterizationSamples = sampleCount,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 1.0f,
                .pSampleMask = nullptr,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE
        };

        vk::PipelineColorBlendAttachmentState colorBlendAttachment{
                .blendEnable = VK_TRUE,
                .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
                .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
                .colorBlendOp = vk::BlendOp::eAdd,
                .srcAlphaBlendFactor = vk::BlendFactor::eOne,
                .dstAlphaBlendFactor = vk::BlendFactor::eZero,
                .alphaBlendOp = vk::BlendOp::eAdd,
                .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                  vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        };

        vk::PipelineColorBlendStateCreateInfo colorBlending{
                .logicOpEnable = VK_FALSE,
                .logicOp = vk::LogicOp::eCopy,
                .attachmentCount = 1,
                .pAttachments = &colorBlendAttachment,
                .blendConstants = {{0.0f, 0.0f, 0.0f, 0.0}}
        };

        auto pushConstants = m_shader->getPushConstants();

        m_layout = m_device.createPipelineLayout({
            .setLayoutCount = static_cast<uint32_t>(layouts.size()),
            .pSetLayouts = layouts.empty() ? nullptr : layouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size()),
            .pPushConstantRanges = pushConstants.data()
        });

        vk::PipelineDepthStencilStateCreateInfo depthStencil{
                .depthTestEnable = VK_TRUE,
                .depthWriteEnable = VK_TRUE,
                .depthCompareOp = vk::CompareOp::eLess,
                .depthBoundsTestEnable = VK_FALSE,
                .stencilTestEnable = VK_FALSE,
                .minDepthBounds = 0.0f,
                .maxDepthBounds = 1.0f,
        };

        auto shaderStages = m_shader->getShaderstages();
        vk::Result result;
        std::tie(result, m_pipeline) = m_device.createGraphicsPipeline(nullptr, {
                .stageCount = static_cast<uint32_t>(shaderStages.size()),
                .pStages = shaderStages.data(),
                .pVertexInputState = &vertexInputInfo,
                .pInputAssemblyState = &inputAssembly,
                .pViewportState = &viewportState,
                .pRasterizationState = &rasterizer,
                .pMultisampleState = &multisampling,
                .pDepthStencilState = &depthStencil,
                .pColorBlendState = &colorBlending,
                .pDynamicState = nullptr,
                .layout = m_layout,
                .renderPass = renderPass,
                .subpass = 0,
                .basePipelineHandle = nullptr,
                .basePipelineIndex = -1
        });

        VK_CHECK_RESULT_HPP(result)
    }

    void GraphicsPipeline::cleanup() {
        m_device.destroy(m_pipeline);
        m_device.destroy(m_layout);
    }

    void GraphicsPipeline::bind(const vk::CommandBuffer& cmdBuffer) {
        cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
    }

    vk::Pipeline GraphicsPipeline::getPipeline() {
        return m_pipeline;
    }

    vk::PipelineLayout GraphicsPipeline::getLayout() {
        return m_layout;
    }

} // namespace vkc