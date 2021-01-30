#include "Renderer.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "Constants.hpp"
#include "Initializers.hpp"
#include "Tools.hpp"
#include "../mesh/Mesh.hpp"
#include "../Utilities.hpp"


namespace core {

    const std::vector<Vertex> vertices = {
            { {-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f} },
            { {0.5f, -0.5f}, {0.0f, 1.0f, 0.0f} },
            { {0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} },
            { {-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f} }
    };

    const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0
    };

    Renderer::Renderer(std::unique_ptr<Window>& window) : m_window(window) {
        init();

        spdlog::info("[Renderer] Initialized");
    }

    Renderer::~Renderer() = default;

    void Renderer::init() {
        VkApplicationInfo appInfo = vk::initializers::applicationInfo();
        appInfo.pApplicationName = "Prototype Action RPG";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.pEngineName = "Custom Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        std::vector<const char*> enabledExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        m_instance.init(appInfo);
        m_instance.createSurface(m_window->m_window, m_surface);
        m_instance.pickPhysicalDevice(m_physicalDevice, m_surface, enabledExtensions);

        m_device = new vk::Device(m_physicalDevice);
        vk::tools::validation(m_device->createLogicalDevice({}, enabledExtensions, vk::validationLayers),
                              "Failed to create logical device");

        m_logicalDevice = m_device->m_logicalDevice;

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

        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createFramebuffers();
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createCommandPool();
        createDescriptorSets();
        recordCommands();
        createSyncObjects();
    }

    void Renderer::clean() {
        vkDeviceWaitIdle(m_logicalDevice);

        cleanSwapChain();

        vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, nullptr);

        vkDestroyBuffer(m_logicalDevice, m_vertexBuffer.m_buffer, nullptr);
        vkFreeMemory(m_logicalDevice, m_vertexBuffer.m_memory, nullptr);
        vkDestroyBuffer(m_logicalDevice, m_indexBuffer.m_buffer, nullptr);
        vkFreeMemory(m_logicalDevice, m_indexBuffer.m_memory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_logicalDevice, m_fences[i], nullptr);
        }

        vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);

        m_device->destroy();
        delete m_device;

        m_instance.destroySurface(m_surface);
        m_instance.destroy();

        spdlog::info("[Renderer] Cleaned");
    }

    void Renderer::draw() {
        drawFrame();
    }

    void Renderer::drawFrame() {
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

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        updateUniformBuffer(indexImage);

        VkSubmitInfo submitInfo = vk::initializers::submitInfo();
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[indexImage];

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_logicalDevice, 1, &m_fences[m_currentFrame]);

        vk::tools::validation(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_fences[m_currentFrame]),
                              "Failed to submit draw command buffer");

        VkSwapchainKHR swapChains[] = { m_swapChain.m_swapChain };

        VkPresentInfoKHR presentInfo = vk::initializers::presentInfo();
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &indexImage;
        presentInfo.pResults = nullptr;

        result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->m_resize) {
            m_window->m_resize = false;
            recreateSwapchain();
        } else if (result != VK_SUCCESS) {
            throw_ex("Failed to present swap chain image");
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapChain.m_format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0,
                dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = vk::initializers::renderPassCreateInfo();
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        vk::tools::validation(vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass),
                              "Failed to create render pass");
    }

    void Renderer::createGraphicsPipeline() {
        auto vertexShaderCode = core::tools::readFile("shaders/shader.vert.spv");
        auto fragmentShaderCode = core::tools::readFile("shaders/shader.frag.spv");

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
        viewport.width = static_cast<float>(m_swapChain.m_extent.width);
        viewport.height = static_cast<float>(m_swapChain.m_extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapChain.m_extent;

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
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
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

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = vk::initializers::pipelineLayoutCreateInfo();
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        vk::tools::validation(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout),
                              "Failed to create pipeline layout");

        VkGraphicsPipelineCreateInfo pipelineInfo = vk::initializers::graphicsPipelineCreateInfo();
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        vk::tools::validation(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline),
                              "Failed to create graphics pipeline");

        vkDestroyShaderModule(m_logicalDevice, vertexShaderModule, nullptr);
        vkDestroyShaderModule(m_logicalDevice, fragmentShaderModule, nullptr);
    }

    void Renderer::createFramebuffers() {
        m_framebuffers.resize(m_swapChain.m_imageCount);

        for (size_t i = 0; i < m_swapChain.m_imageCount; ++i) {
            std::array<VkImageView, 1> attachments = {
                    m_swapChain.m_buffers[i].view
            };

            VkFramebufferCreateInfo framebufferInfo = vk::initializers::framebufferCreateInfo();
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_swapChain.m_extent.width;
            framebufferInfo.height = m_swapChain.m_extent.height;
            framebufferInfo.layers = 1;

            vk::tools::validation(vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_framebuffers[i]),
                                  "Failed to create framebuffer");
        }
    }

    void Renderer::createCommandPool() {
        m_commandPool = m_device->createCommandPool(m_device->m_queueFamilyIndices.graphics);

        m_commandBuffers.resize(m_swapChain.m_imageCount);

        for (int i = 0; i < m_swapChain.m_imageCount; ++i) {
            m_commandBuffers[i] = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool);
        }
    }

    void Renderer::createSyncObjects() {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_fences.resize(MAX_FRAMES_IN_FLIGHT);
        m_imageFences.resize(m_swapChain.m_imageCount, VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = vk::initializers::semaphoreCreateInfo();

        VkFenceCreateInfo fenceInfo = vk::initializers::fenceCreateInfo();
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vk::tools::validation(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]),
                                  "Failed to create semaphores");

            vk::tools::validation(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]),
                                  "Failed to create semaphores");

            vk::tools::validation(vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &m_fences[i]),
                                  "Failed to create a fence");
        }
    }

    void Renderer::recordCommands() {
        VkClearValue clearColors[] = { {0.0f, 0.0f, 0.0f, 1.0f} };
        VkRenderPassBeginInfo renderPassInfo = vk::initializers::renderPassBeginInfo();

        for (size_t i = 0; i < m_commandBuffers.size(); ++i) {
            VkCommandBufferBeginInfo beginInfo = vk::initializers::commandBufferBeginInfo();
            beginInfo.pInheritanceInfo = nullptr;

            vk::tools::validation(vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo),
                                  "Failed to begin recording command buffer");
            {
                renderPassInfo.renderPass = m_renderPass;
                renderPassInfo.framebuffer = m_framebuffers[i];
                renderPassInfo.renderArea.offset = { 0, 0 };
                renderPassInfo.renderArea.extent = m_swapChain.m_extent;
                renderPassInfo.clearValueCount = 1;
                renderPassInfo.pClearValues = clearColors;

                vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                {
                    vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

                    VkBuffer vertexBuffer[] = { m_vertexBuffer.m_buffer };
                    VkDeviceSize offsets[] = { 0 };
                    vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffer, offsets);
                    vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT16);

                    vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);
                    vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(indices.size()),
                                     1, 0, 0, 0);
                }
                vkCmdEndRenderPass(m_commandBuffers[i]);
            }
            vk::tools::validation(vkEndCommandBuffer(m_commandBuffers[i]),
                           "Failed to record command buffer");
        }
    }

    void Renderer::recreateSwapchain() {
        // TODO: Optimize window resize
        while (m_window->m_size.width == 0 || m_window->m_size.height == 0)  {
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_logicalDevice);

        cleanSwapChain();

        m_swapChain.create(m_windowSize.width, m_windowSize.height, m_device->m_queueFamilyIndices.graphics, m_device->m_queueFamilyIndices.present);
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();

        for (int i = 0; i < m_swapChain.m_imageCount; ++i) {
            m_commandBuffers[i] = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool);
        }

        recordCommands();
    }

    void Renderer::cleanSwapChain() {
        vkFreeCommandBuffers(m_logicalDevice, m_commandPool,
                             static_cast<uint64_t>(m_commandBuffers.size()), m_commandBuffers.data());

        for (auto & framebuffer : m_framebuffers) {
            vkDestroyFramebuffer(m_logicalDevice, framebuffer, nullptr);
        }

        vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
        vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);

        m_swapChain.cleanup();

        for (size_t i = 0; i < m_swapChain.m_imageCount; i++) {
            vkDestroyBuffer(m_logicalDevice, m_uniformBuffers[i].m_buffer, nullptr);
            vkFreeMemory(m_logicalDevice, m_uniformBuffers[i].m_memory, nullptr);
        }

        vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    }

    void Renderer::createVertexBuffer() {
        VkDeviceSize size = sizeof(Vertex) * vertices.size();
        vk::Buffer stagingBuffer;

        m_device->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               &stagingBuffer,
                               size,
                               (void*)vertices.data());

        m_device->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               &m_vertexBuffer,
                               size);

        m_device->copyBuffer(&stagingBuffer, &m_vertexBuffer, m_graphicsQueue);

        vkDestroyBuffer(m_logicalDevice, stagingBuffer.m_buffer, nullptr);
        vkFreeMemory(m_logicalDevice, stagingBuffer.m_memory, nullptr);
    }

    void Renderer::createIndexBuffer() {
        VkDeviceSize size = sizeof(uint16_t) * indices.size();
        vk::Buffer stagingBuffer;

        m_device->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               &stagingBuffer,
                               size,
                               (void*)indices.data());

        m_device->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               &m_indexBuffer,
                               size);

        m_device->copyBuffer(&stagingBuffer, &m_indexBuffer, m_graphicsQueue);

        vkDestroyBuffer(m_logicalDevice, stagingBuffer.m_buffer, nullptr);
        vkFreeMemory(m_logicalDevice, stagingBuffer.m_memory, nullptr);
    }

    void Renderer:: createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo = vk::initializers::descriptorSetLayoutCreateInfo();
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        vk::tools::validation(vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, &m_descriptorSetLayout),
                              "Failed to create descriptor set layout");
    }

    void Renderer::createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_uniformBuffers.resize(m_swapChain.m_imageCount);

        for (size_t i = 0; i < m_swapChain.m_imageCount; ++i) {
            m_device->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   &m_uniformBuffers[i],
                                   bufferSize);
        }
    }

    void Renderer::updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f));

        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                               glm::vec3(0.0f, 0.0f, 1.0f));

        ubo.proj = glm::perspective(glm::radians(45.0f), m_window->aspect(), 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        m_uniformBuffers[currentImage].map(sizeof(ubo), 0);
        m_uniformBuffers[currentImage].copyTo(&ubo, sizeof(ubo));
        m_uniformBuffers[currentImage].unmap();
    }

    void Renderer::createDescriptorPool() {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = m_swapChain.m_imageCount;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = m_swapChain.m_imageCount;
        poolInfo.flags = 0;

        vk::tools::validation(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool),
                              "Failed to create descriptor pool");
    }

    void Renderer::createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(m_swapChain.m_imageCount, m_descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo = vk::initializers::descriptorSetAllocateInfo();
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = m_swapChain.m_imageCount;
        allocInfo.pSetLayouts = layouts.data();

        m_descriptorSets.resize(m_swapChain.m_imageCount);
        vk::tools::validation(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, m_descriptorSets.data()),
                              "Failed to allocate descriptor sets");

        for (size_t i = 0; i < m_swapChain.m_imageCount; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i].m_buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite = vk::initializers::writeDescriptorSet();
            descriptorWrite.dstSet = m_descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr;
            descriptorWrite.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(m_logicalDevice, 1, &descriptorWrite, 0, nullptr);
        }
    }

} // End namespace core