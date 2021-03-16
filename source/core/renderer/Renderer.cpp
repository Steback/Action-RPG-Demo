#include "Renderer.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Initializers.hpp"
#include "Tools.hpp"
#include "../Constants.hpp"
#include "../Utilities.hpp"
#include "../components/Transform.hpp"
#include "../components/MeshModel.hpp"
#include "../resources/Model.hpp"


namespace core {

    Renderer::Renderer(std::unique_ptr<Window>& window, VkInstance instance, const std::string& appName, vk::Device* device, VkSurfaceKHR surface)
            : m_window(window), m_device(device) {
        m_logicalDevice = m_device->m_logicalDevice;
        m_physicalDevice = m_device->m_physicalDevice;
        m_surface = surface;
        m_msaaSamples = m_device->getMaxUsableSampleCount();

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_device->m_queueFamilyIndices.graphics, m_surface, &presentSupport);

        if (presentSupport) {
            m_device->m_queueFamilyIndices.present = m_device->m_queueFamilyIndices.graphics;
        }

        vkGetDeviceQueue(m_logicalDevice, m_device->m_queueFamilyIndices.graphics, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_logicalDevice, m_device->m_queueFamilyIndices.present, 0, &m_presentQueue);

        m_windowSize = m_window->getSize();
        m_swapChain.connect(m_physicalDevice, m_logicalDevice, m_surface);
        m_swapChain.create(m_windowSize.width, m_windowSize.height, m_device->m_queueFamilyIndices.graphics, m_device->m_queueFamilyIndices.present);

        m_depthFormat= vk::tools::findSupportedFormat(m_physicalDevice,
                                                      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                                      VK_IMAGE_TILING_OPTIMAL,
                                                      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        m_ui = core::UIImGui(m_swapChain, m_device, m_window->getWindow(), instance, m_graphicsQueue);

    }

    Renderer::~Renderer() = default;

    void Renderer::init(core::ResourceManager* resourceManager, bool drawGrid) {
        m_resourceManager = resourceManager;
        m_drawGrid = drawGrid;

        createCommandPool();
        createRenderPass();
        createDescriptorSetLayout();
        createPushConstants();
        createGraphicsPipeline();
        createMsaaResources();
        createDepthResources();
        createFramebuffers();
        createDescriptorPool();
        createDescriptorSets();
        createSyncObjects();

        spdlog::info("[Renderer] Initialized");
    }

    void Renderer::cleanup() {

        UIImGui::cleanupImGui();

        cleanSwapChain();

        m_ui.cleanup();

        vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_logicalDevice, m_fences[i], nullptr);
        }

        vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);

        spdlog::info("[Renderer] Cleaned");
    }

    void Renderer::draw(entt::registry& registry) {
        drawFrame(registry);
    }

    void Renderer::drawFrame(entt::registry& registry) {
        vkWaitForFences(m_logicalDevice, 1, &m_fences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

        uint32_t indexImage;
        VkResult result = m_swapChain.acquireNextImage(m_imageAvailableSemaphores[m_currentFrame], &indexImage);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapchain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw_ex("Failed to acquire swap chain image");
        }

        if (m_imageFences[indexImage] != VK_NULL_HANDLE) {
            vkWaitForFences(m_logicalDevice, 1, &m_imageFences[indexImage], VK_TRUE, std::numeric_limits<uint64_t>::max());
        }

        m_imageFences[indexImage] = m_fences[m_currentFrame];

        recordCommands(indexImage, registry);
        m_ui.recordCommands(indexImage, m_swapChain.getExtent());

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        std::array<VkCommandBuffer, 2> cmdBuffers = {
                m_commandBuffers[indexImage],
                m_ui.getCommandBuffer(indexImage)
        };

        VkSubmitInfo submitInfo = vk::initializers::submitInfo();
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
        submitInfo.pCommandBuffers = cmdBuffers.data();

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_logicalDevice, 1, &m_fences[m_currentFrame]);

        VK_CHECK_RESULT(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_fences[m_currentFrame]));

        VkSwapchainKHR swapChains[] = { m_swapChain.getSwapChain() };

        VkPresentInfoKHR presentInfo = vk::initializers::presentInfo();
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &indexImage;
        presentInfo.pResults = nullptr;

        result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->resize()) {
            m_window->resize() = false;
            recreateSwapchain();
        } else if (result != VK_SUCCESS) {
            throw_ex("Failed to present swap chain image");
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapChain.getFormat();
        colorAttachment.samples = m_msaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = m_depthFormat;
        depthAttachment.samples = m_msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = m_swapChain.getFormat();
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.pResolveAttachments = &colorAttachmentResolveRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
        VkRenderPassCreateInfo renderPassInfo = vk::initializers::renderPassCreateInfo();
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VK_CHECK_RESULT(vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass));
    }

    void Renderer::createGraphicsPipeline() {
        auto vertexShaderCode = core::tools::readFile("shaders/model.vert.spv");
        auto fragmentShaderCode = core::tools::readFile("shaders/model.frag.spv");

        VkShaderModule vertexShaderModule = vk::tools::loadShader(vertexShaderCode, m_logicalDevice);
        VkShaderModule fragmentShaderModule = vk::tools::loadShader(fragmentShaderCode, m_logicalDevice);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = vk::initializers::pipelineShaderStageCreateInfo();
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertexShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = vk::initializers::pipelineShaderStageCreateInfo();
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragmentShaderModule;
        fragShaderStageInfo.pName = "main";

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
                vertShaderStageInfo,
                fragShaderStageInfo
        };

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = vk::initializers::pipelineVertexInputStateCreateInfo();
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = vk::initializers::pipelineInputAssemblyStateCreateInfo();
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChain.getExtent().width);
        viewport.height = static_cast<float>(m_swapChain.getExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapChain.getExtent();

        VkPipelineViewportStateCreateInfo viewportState = vk::initializers::pipelineViewportStateCreateInfo();
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = vk::initializers::pipelineRasterizationStateCreateInfo();
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;
        rasterizer.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampling = vk::initializers::pipelineMultisampleStateCreateInfo();
        multisampling.rasterizationSamples = m_msaaSamples;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlending = vk::initializers::pipelineColorBlendStateCreateInfo();
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::array<VkDescriptorSetLayout, 2> layouts = {
                m_descriptorSetLayout,
                m_resourceManager->getTextureDescriptorSetLayout()
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = vk::initializers::pipelineLayoutCreateInfo();
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &m_mvpRange;

        VK_CHECK_RESULT(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

        VkPipelineDepthStencilStateCreateInfo depthStencil = vk::initializers::pipelineDepthStencilStateCreateInfo();
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        VkGraphicsPipelineCreateInfo pipelineInfo = vk::initializers::graphicsPipelineCreateInfo();
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline));

        vkDestroyShaderModule(m_logicalDevice, vertexShaderModule, nullptr);
        vkDestroyShaderModule(m_logicalDevice, fragmentShaderModule, nullptr);

        if (m_drawGrid) {
            createGridPipeline(pipelineInfo);
        }
    }

    void Renderer::createFramebuffers() {
        m_framebuffers.resize(m_swapChain.getImageCount());

        for (size_t i = 0; i < m_swapChain.getImageCount(); ++i) {
            std::array<VkImageView, 3> attachments = {
                    m_colorImage.getView(),
                    m_depthBuffer.getView(),
                    m_swapChain.getImageView(i)
            };

            VkFramebufferCreateInfo framebufferInfo = vk::initializers::framebufferCreateInfo();
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_swapChain.getExtent().width;
            framebufferInfo.height = m_swapChain.getExtent().height;
            framebufferInfo.layers = 1;

            VK_CHECK_RESULT(vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_framebuffers[i]));
        }
    }

    void Renderer::createCommandPool() {
        m_commandPool = m_device->createCommandPool(m_device->m_queueFamilyIndices.graphics);

        m_commandBuffers.resize(m_swapChain.getImageCount());

        for (int i = 0; i < m_swapChain.getImageCount(); ++i) {
            m_commandBuffers[i] = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool);
        }
    }

    void Renderer::createSyncObjects() {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_fences.resize(MAX_FRAMES_IN_FLIGHT);
        m_imageFences.resize(m_swapChain.getImageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = vk::initializers::semaphoreCreateInfo();

        VkFenceCreateInfo fenceInfo = vk::initializers::fenceCreateInfo();
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            VK_CHECK_RESULT(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]));

            VK_CHECK_RESULT(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]));

            VK_CHECK_RESULT(vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &m_fences[i]));
        }
    }

    void Renderer::recordCommands(uint32_t indexImage, entt::registry& registry) {
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.24f, 0.24f, 0.24f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassInfo = vk::initializers::renderPassBeginInfo();
        auto view = registry.view<core::MeshModel, core::Transform>();

        VkCommandBufferBeginInfo beginInfo = vk::initializers::commandBufferBeginInfo();
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK_RESULT(vkBeginCommandBuffer(m_commandBuffers[indexImage], &beginInfo));
        {
            renderPassInfo.renderPass = m_renderPass;
            renderPassInfo.framebuffer = m_framebuffers[indexImage];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = m_swapChain.getExtent();
            renderPassInfo.renderArea.extent.width = m_swapChain.getExtent().width;
            renderPassInfo.renderArea.extent.height = m_swapChain.getExtent().height;
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(m_commandBuffers[indexImage], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            {
                vkCmdBindPipeline(m_commandBuffers[indexImage], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

                for (auto& entity : view) {
                    auto& transform = view.get<core::Transform>(entity);
                    auto& meshModel = view.get<core::MeshModel>(entity);
                    core::Model& model = m_resourceManager->getModel(meshModel.getModelID());

                    for (auto& node : model.getNodes()) {
                        glm::mat4 nodeMatrix = node.matrix;
                        core::Model::Node* parent = node.parent;

                        while (parent) {
                            nodeMatrix = parent->matrix * nodeMatrix;
                            parent = parent->parent;
                        }

                        m_mvp.model = nodeMatrix * transform.worldTransformMatrix();

                        if (node.mesh > 0) {
                            auto& mesh = m_resourceManager->getMesh(node.mesh);

                            VkBuffer vertexBuffer[] = {mesh.getVertexBuffer()};
                            VkDeviceSize offsets[] = {0};
                            vkCmdBindVertexBuffers(m_commandBuffers[indexImage], 0, 1, vertexBuffer, offsets);
                            vkCmdBindIndexBuffer(m_commandBuffers[indexImage], mesh.getIndexBuffer(), 0,
                                                 VK_INDEX_TYPE_UINT32);

                            vkCmdPushConstants(m_commandBuffers[indexImage], m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
                                               0,
                                               sizeof(MVP), &m_mvp);

                            std::array<VkDescriptorSet, 2> descriptorSetGroup = {
                                    m_descriptorSets[indexImage],
                                    m_resourceManager->getTexture(mesh.getTextureId()).getDescriptorSet()
                            };

                            vkCmdBindDescriptorSets(m_commandBuffers[indexImage], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                    m_pipelineLayout,
                                                    0, static_cast<uint32_t>(descriptorSetGroup.size()),
                                                    descriptorSetGroup.data(), 0, nullptr);

                            vkCmdBindDescriptorSets(m_commandBuffers[indexImage], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                    m_pipelineLayout, 0, 1, &m_descriptorSets[indexImage], 0, nullptr);

                            vkCmdDrawIndexed(m_commandBuffers[indexImage], mesh.getIndexCount(),
                                             1, 0, 0, 0);
                        }
                    }
                }

                if (m_drawGrid) {
                    vkCmdBindPipeline(m_commandBuffers[indexImage], VK_PIPELINE_BIND_POINT_GRAPHICS, m_gridPipeline);

                    m_mvp.model = glm::mat4(1.0f);

                    vkCmdPushConstants(m_commandBuffers[indexImage], m_gridPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                       sizeof(MVP), &m_mvp);

                    vkCmdDraw(m_commandBuffers[indexImage], 6, 1, 0, 0);
                }
            }
            vkCmdEndRenderPass(m_commandBuffers[indexImage]);
        }
        VK_CHECK_RESULT(vkEndCommandBuffer(m_commandBuffers[indexImage]));
    }

    void Renderer::recreateSwapchain() {
        // TODO: Optimize window resize
        while (m_windowSize.width == 0 || m_windowSize.height == 0)  {
            m_windowSize = m_window->getSize();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_logicalDevice);

        cleanSwapChain();

        m_swapChain.create(m_windowSize.width, m_windowSize.height, m_device->m_queueFamilyIndices.graphics, m_device->m_queueFamilyIndices.present);
        createRenderPass();
        createGraphicsPipeline();
        createMsaaResources();
        createDepthResources();
        createFramebuffers();
        createDescriptorPool();
        createDescriptorSets();

        for (int i = 0; i < m_swapChain.getImageCount(); ++i) {
            m_commandBuffers[i] = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool);
        }

        m_ui.resize(m_swapChain);
        m_resourceManager->recreateResources();
    }

    void Renderer::cleanSwapChain() {
        m_depthBuffer.cleanup(m_logicalDevice);
        m_colorImage.cleanup(m_logicalDevice);

        m_ui.cleanupResources();

        for (auto & framebuffer : m_framebuffers) {
            vkDestroyFramebuffer(m_logicalDevice, framebuffer, nullptr);
        }

        vkFreeCommandBuffers(m_logicalDevice, m_commandPool, static_cast<uint64_t>(m_commandBuffers.size()),
                             m_commandBuffers.data());

        if (m_drawGrid) {
            vkDestroyPipeline(m_logicalDevice, m_gridPipeline, nullptr);
            vkDestroyPipelineLayout(m_logicalDevice, m_gridPipelineLayout, nullptr);
        }

        vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
        vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);

        m_swapChain.cleanup();

        vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
        m_resourceManager->cleanupResources();
    }

    void Renderer:: createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo uboLayoutInfo = vk::initializers::descriptorSetLayoutCreateInfo();
        uboLayoutInfo.bindingCount = 1;
        uboLayoutInfo.pBindings = &uboLayoutBinding;

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_logicalDevice, &uboLayoutInfo, nullptr, &m_descriptorSetLayout));
    }

    void Renderer::createDescriptorPool() {
        VkDescriptorPoolSize descriptorPoolSize{};
        descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorPoolSize.descriptorCount = m_swapChain.getImageCount();

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = vk::initializers::descriptorPoolCreateInfo();
        descriptorPoolCreateInfo.poolSizeCount = 1;
        descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
        descriptorPoolCreateInfo.maxSets = m_swapChain.getImageCount();
        descriptorPoolCreateInfo.flags = 0;

        VK_CHECK_RESULT(vkCreateDescriptorPool(m_logicalDevice, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool));
    }

    void Renderer::createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(m_swapChain.getImageCount(), m_descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo = vk::initializers::descriptorSetAllocateInfo();
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = m_swapChain.getImageCount();
        allocInfo.pSetLayouts = layouts.data();

        m_descriptorSets.resize(m_swapChain.getImageCount());

        VK_CHECK_RESULT(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, m_descriptorSets.data()));
    }

    void Renderer::createDepthResources() {
        VkImageCreateInfo imageInfo = vk::initializers::imageCreateInfo();
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_swapChain.getExtent().width;
        imageInfo.extent.height = m_swapChain.getExtent().height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = m_depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = m_msaaSamples;
        imageInfo.flags = 0;

        m_depthBuffer = vk::Image(m_logicalDevice, imageInfo);

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_logicalDevice, m_depthBuffer.getImage(), &memoryRequirements);

        auto memType = m_device->getMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_depthBuffer.bind(m_logicalDevice, memType, memoryRequirements.size, VK_IMAGE_ASPECT_DEPTH_BIT);

        m_device->transitionImageLayout(m_depthBuffer.getImage(), m_depthBuffer.getFormat(), m_graphicsQueue,
                                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    void Renderer::createMsaaResources() {
        VkFormat colorFormat = m_swapChain.getFormat();

        VkImageCreateInfo imageInfo = vk::initializers::imageCreateInfo();
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_swapChain.getExtent().width;
        imageInfo.extent.height = m_swapChain.getExtent().height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = colorFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = m_msaaSamples;
        imageInfo.flags = 0;

        m_colorImage = vk::Image(m_logicalDevice, imageInfo);

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_logicalDevice, m_colorImage.getImage(), &memoryRequirements);

        auto memType = m_device->getMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_colorImage.bind(m_logicalDevice, memType, memoryRequirements.size, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void Renderer::createPushConstants() {
        m_mvpRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        m_mvpRange.offset = 0;
        m_mvpRange.size = sizeof(MVP);
    }

    void Renderer::createGridPipeline(VkGraphicsPipelineCreateInfo& createInfo) {
        auto vertexShaderCode = core::tools::readFile("shaders/grid.vert.spv");
        auto fragmentShaderCode = core::tools::readFile("shaders/grid.frag.spv");

        VkShaderModule vertShaderModule = vk::tools::loadShader(vertexShaderCode, m_logicalDevice);
        VkShaderModule fragShaderModule = vk::tools::loadShader(fragmentShaderCode, m_logicalDevice);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = vk::initializers::pipelineShaderStageCreateInfo();
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = vk::initializers::pipelineShaderStageCreateInfo();
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
                vertShaderStageInfo,
                fragShaderStageInfo
        };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = vk::initializers::pipelineVertexInputStateCreateInfo();
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;

        createInfo.pVertexInputState = &vertexInputInfo;

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::initializers::pipelineLayoutCreateInfo();
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &m_mvpRange;

        VK_CHECK_RESULT(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &m_gridPipelineLayout));

        createInfo.pStages = shaderStages.data();
        createInfo.layout = m_gridPipelineLayout;

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_logicalDevice, nullptr, 1, &createInfo, nullptr, &m_gridPipeline));

        vkDestroyShaderModule(m_logicalDevice, vertShaderModule, nullptr);
        vkDestroyShaderModule(m_logicalDevice, fragShaderModule, nullptr);
    }

    void Renderer::updateVP(const glm::mat4& view, const glm::mat4& proj) {
        m_mvp.view = view;
        m_mvp.proj = proj;
    }

    VkPhysicalDevice &Renderer::getPhysicalDevice() {
        return m_physicalDevice;
    }

    VkQueue &Renderer::getGraphicsQueue() {
        return m_graphicsQueue;
    }

} // End namespace core