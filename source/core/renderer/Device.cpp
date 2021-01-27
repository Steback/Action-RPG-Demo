#include "Device.hpp"

#include <set>
#include <array>

#include "Debug.hpp"
#include "Utils.hpp"


namespace vk {

    Device::Device() = default;

    Device::~Device() = default;

    VkDevice &Device::operator*() {
        return m_device;
    }

    void Device::init(const PhysicalDevice& physicalDevice, QueueFamilyIndices indices, VkQueue& graphicsQueue,
                      VkQueue& presentQueue) {
        m_physicalDevice = physicalDevice;
        m_familyIndices = indices;

        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
        std::set<uint32_t> uniqueQueueFamilies = {
                indices.graphics.value(),
                indices.present.value()
        };

        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        queueCreateInfos.reserve(uniqueQueueFamilies.size());

        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = initializers::deviceQueueCreateInfo();
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkDeviceCreateInfo deviceCreateInfo = initializers::deviceCreateInfo();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0;
        deviceCreateInfo.ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures,

                validation(vkCreateDevice(m_physicalDevice.device, &deviceCreateInfo, nullptr, &m_device),
                           "Failed to create logical device");

        vkGetDeviceQueue(m_device, indices.graphics.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(m_device, indices.present.value(), 0, &presentQueue);
    }

    void Device::destroy() {
        vkDestroyDevice(m_device, nullptr);
    }

    void Device::waitIdle() {
        vkDeviceWaitIdle(m_device);
    }

    void Device::createSwapChain(SwapChain &swapChain, const core::WindowSize& windowSize, VkSurfaceKHR surface) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice.device, surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtend(windowSize, swapChainSupport.capabilities);

        swapChain.imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 &&
                swapChain.imageCount > swapChainSupport.capabilities.maxImageCount) {
            swapChain.imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = initializers::swapchainCreateInfo();
        createInfo.surface = surface;
        createInfo.minImageCount = swapChain.imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = nullptr;

        std::vector<uint32_t> queueFamilyIndices = {
                m_familyIndices.graphics.value(), m_familyIndices.present.value()
        };

        if (m_familyIndices.graphics != m_familyIndices.present) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT,
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        validation(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &swapChain.swapchain),
                   "Failed to create swap chain");

        vkGetSwapchainImagesKHR(m_device, swapChain.swapchain, &swapChain.imageCount, nullptr);
        swapChain.images.resize(swapChain.imageCount);
        vkGetSwapchainImagesKHR(m_device, swapChain.swapchain, &swapChain.imageCount, swapChain.images.data());

        swapChain.extent = extent;
        swapChain.format = surfaceFormat.format;
    }

    void Device::destroySwapChain(SwapChain& swapChain) {
        vkDestroySwapchainKHR(m_device, swapChain.swapchain, nullptr);
    }

    void Device::createImageViews(SwapChain &swapChain) {
        swapChain.imageViews.resize(swapChain.images.size());

        for (size_t i = 0; i < swapChain.images.size(); ++i) {
            VkImageViewCreateInfo createInfo = initializers::imageViewCreateInfo();
            createInfo.image = swapChain.images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChain.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            validation(vkCreateImageView(m_device, &createInfo, nullptr, &swapChain.imageViews[i]),
                       "Failed to create image views");
        }
    }

    void Device::destroyImageViews(SwapChain &swapChain) {
        for (auto& imageView : swapChain.imageViews) {
            vkDestroyImageView(m_device, imageView, nullptr);
        }
    }

    void Device::createGraphicsPipeline(VkPipeline& graphicsPipeline, VkPipelineLayout& pipelineLayout,
                                        const VkExtent2D& swapChainExtend, const VkRenderPass& renderPass) {
        auto vertexShaderCode = readFile("shaders/shader.vert.spv");
        auto fragmentShaderCode = readFile("shaders/shader.frag.spv");

        VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
        VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = initializers::pipelineShaderStageCreateInfo();
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertexShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = initializers::pipelineShaderStageCreateInfo();
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragmentShaderModule;
        fragShaderStageInfo.pName = "main";

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
                vertShaderStageInfo,
                fragShaderStageInfo
        };

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = initializers::pipelineVertexInputStateCreateInfo();
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = initializers::pipelineInputAssemblyStateCreateInfo();
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtend.width);
        viewport.height = static_cast<float>(swapChainExtend.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtend;

        VkPipelineViewportStateCreateInfo viewportState = initializers::pipelineViewportStateCreateInfo();
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = initializers::pipelineRasterizationStateCreateInfo();
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;
        rasterizer.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampling = initializers::pipelineMultisampleStateCreateInfo();
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

        VkPipelineColorBlendStateCreateInfo colorBlending = initializers::pipelineColorBlendStateCreateInfo();
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = initializers::pipelineLayoutCreateInfo();
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        validation(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &pipelineLayout),
                   "Failed to create pipeline layout");

        VkGraphicsPipelineCreateInfo pipelineInfo = initializers::graphicsPipelineCreateInfo();
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
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        validation(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline),
                   "Failed to create graphics pipeline");

        destroyShaderModule(vertexShaderModule);
        destroyShaderModule(fragmentShaderModule);
    }

    void Device::destroyGraphicsPipeline(VkPipeline& graphicsPipeline, VkPipelineLayout &pipelineLayout) {
        vkDestroyPipeline(m_device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr);
    }

    VkShaderModule Device::createShaderModule(const std::vector<char> &code) {
        VkShaderModuleCreateInfo createInfo = initializers::shaderModuleCreateInfo();
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;

        validation(vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule),
                   "Failed to create shader module");

        return shaderModule;
    }

    void Device::destroyShaderModule(VkShaderModule &shader) {
        vkDestroyShaderModule(m_device, shader, nullptr);
    }

    void Device::createRenderPass(VkRenderPass& renderPass, const VkFormat& swapChainFormat) {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainFormat;
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

        VkRenderPassCreateInfo renderPassInfo = initializers::renderPassCreateInfo();
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        validation(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &renderPass),
                   "Failed to create render pass");
    }

    void Device::destroyRenderPass(VkRenderPass &renderPass) {
        vkDestroyRenderPass(m_device, renderPass, nullptr);
    }

    void Device::createFramebuffers(SwapChain& swapChain, const VkRenderPass& renderPass) {
        swapChain.framebuffers.resize(swapChain.imageViews.size());

        for (size_t i = 0; i < swapChain.imageViews.size(); ++i) {
            std::array<VkImageView, 1> attachments = {
                swapChain.imageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo = initializers::framebufferCreateInfo();
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChain.extent.width;
            framebufferInfo.height = swapChain.extent.height;
            framebufferInfo.layers = 1;

            validation(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &swapChain.framebuffers[i]),
                       "Failed to create framebuffer");
        }
    }

    void Device::destroyFramebuffers(std::vector<VkFramebuffer>& swapChainFramebuffers) {
        for (auto & framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }
    }

    void Device::createCommandPool(VkCommandPool &commandPool, VkSurfaceKHR const &surface,
                                   const VkCommandPoolCreateFlags& flags) {
        VkCommandPoolCreateInfo poolInfo = initializers::commandPoolCreateInfo();
        poolInfo.flags = flags;
        poolInfo.queueFamilyIndex = m_familyIndices.graphics.value();

        validation(vkCreateCommandPool(m_device, &poolInfo, nullptr, &commandPool),
                   "Failed to create command pool");
    }

    void Device::destroyCommandPool(VkCommandPool &commandPool) {
        vkDestroyCommandPool(m_device, commandPool, nullptr);
    }

    void Device::createCommandBuffers(CommandPool& commandPool, const std::vector<VkFramebuffer>& swapChainFramebuffers) {
        commandPool.buffers.resize(swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo = initializers::commandBufferAllocateInfo();
        allocInfo.commandPool = commandPool.pool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandPool.buffers.size());

        validation(vkAllocateCommandBuffers(m_device, &allocInfo, commandPool.buffers.data()),
                   "Failed to allocate command buffers");
    }

    void Device::freeCommandBuffers(CommandPool& commandPool) {
        vkFreeCommandBuffers(m_device, commandPool.pool, static_cast<uint32_t>(commandPool.buffers.size()),
                             commandPool.buffers.data());
    }

    void Device::createSemaphore(VkSemaphore& semaphore) {
        VkSemaphoreCreateInfo semaphoreInfo = initializers::semaphoreCreateInfo();

        validation(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &semaphore),
                   "Failed to create semaphores");
    }

    void Device::destroySemaphore(VkSemaphore& semaphore) {
        vkDestroySemaphore(m_device, semaphore, nullptr);
    }

    VkResult Device::acquireNextImage(uint32_t& imageIndex, const VkSwapchainKHR& swapchain,
                                  const VkSemaphore& imageAvailableSemaphore) {
        return vkAcquireNextImageKHR(m_device, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore,
                                     VK_NULL_HANDLE, &imageIndex);
    }

    void Device::createFence(VkFence& fence) {
        VkFenceCreateInfo fenceInfo = initializers::fenceCreateInfo();
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        validation(vkCreateFence(m_device, &fenceInfo, nullptr, &fence),
                   "Failed to create a fence");
    }

    void Device::destroyFence(VkFence &fence) {
        vkDestroyFence(m_device, fence, nullptr);
    }

    void Device::waitForFence(const VkFence& fence) {
        vkWaitForFences(m_device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    }

    void Device::resetFence(VkFence const &fence) {
        vkResetFences(m_device, 1, &fence);
    }

    void Device::createBuffer(Buffer& buffer, const VkBufferUsageFlags& flags, const VkMemoryPropertyFlags& properties,
                              const VkDeviceSize& size, const VkSharingMode& sharingMode) {
        VkBufferCreateInfo bufferInfo = initializers::bufferCreateInfo();
        bufferInfo.size = size;
        bufferInfo.usage = flags;
        bufferInfo.sharingMode = sharingMode;

        validation(vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer.buffer),
                   "Failed to create vertex buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, buffer.buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = initializers::memoryAllocateInfo();
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(m_physicalDevice.device, memRequirements.memoryTypeBits, properties);

        validation(vkAllocateMemory(m_device, &allocInfo, nullptr, &buffer.deviceMemory),
                   "Failed to allocate vertex buffer memory");

        vkBindBufferMemory(m_device, buffer.buffer, buffer.deviceMemory, 0);
    }

    void Device::destroyBuffer(Buffer& buffer) {
        vkDestroyBuffer(m_device, buffer.buffer, nullptr);
        vkFreeMemory(m_device, buffer.deviceMemory, nullptr);
    }

    void Device::copyBuffer(VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkCommandPool &commandPool, VkQueue& queue,
                            const VkDeviceSize& size) {
        VkCommandBufferAllocateInfo allocInfo = initializers::commandBufferAllocateInfo();
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = initializers::commandBufferBeginInfo();
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        {
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = size;

            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        }
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = initializers::submitInfo();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(m_device, commandPool, 1, &commandBuffer);
    }

} // End namespace vk
