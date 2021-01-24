#include "Device.hpp"

#include <set>
#include <array>

#include "Debug.hpp"
#include "Utils.hpp"


namespace vk {

    Device::Device() = default;

    Device::~Device() = default;

    void Device::init(const PhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkQueue& graphicsQueue,
                      VkQueue& presentQueue) {
        mPhysicalDevice = physicalDevice;
        QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice.device, surface);
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
        std::set<uint32_t> uniqueQueueFamilies = {
                indices.graphics.value(),
                indices.present.value() };

        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        queueCreateInfos.reserve(uniqueQueueFamilies.size());

        for (uint32_t queueFamily : uniqueQueueFamilies) {
            queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queueFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority });
        }

        VkDeviceCreateInfo deviceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.data(),
                .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
                .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
                .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
                .ppEnabledExtensionNames = deviceExtensions.data(),
                .pEnabledFeatures = &physicalDeviceFeatures,
        };

        resultValidation(vkCreateDevice(mPhysicalDevice.device, &deviceCreateInfo, nullptr, &mDevice),
                         "Failed to create logical device");

        vkGetDeviceQueue(mDevice, indices.graphics.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(mDevice, indices.present.value(), 0, &presentQueue);
    }

    void Device::destroy() {
        vkDestroyDevice(mDevice, nullptr);
    }

    void Device::waitIdle() {
        vkDeviceWaitIdle(mDevice);
    }

    void Device::createSwapChain(SwapChain &swapChain, const core::WindowSize& windowSize, VkSurfaceKHR surface, bool recreate) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(mPhysicalDevice.device, surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtend(windowSize, swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = swapChainSupport.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = nullptr
        };

        QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice.device, surface);
        std::vector<uint32_t> queueFamilyIndices = {indices.graphics.value(), indices.present.value() };

        if (indices.graphics != indices.present) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT,
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        resultValidation(vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &swapChain.mSwapChain),
                         "Failed to create swap chain");

        vkGetSwapchainImagesKHR(mDevice, swapChain.mSwapChain, &imageCount, nullptr);
        swapChain.mImages.resize(imageCount);
        vkGetSwapchainImagesKHR(mDevice, swapChain.mSwapChain, &imageCount, swapChain.mImages.data());

        swapChain.mExtent = extent;
        swapChain.mImageFormat = surfaceFormat.format;
    }

    void Device::destroySwapChain(SwapChain& swapChain) {
        vkDestroySwapchainKHR(mDevice, swapChain.mSwapChain, nullptr);
    }

    void Device::createImageViews(SwapChain &swapChain) {
        swapChain.mImageViews.resize(swapChain.mImages.size());

        for (size_t i = 0; i < swapChain.mImages.size(); ++i) {
            VkImageViewCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapChain.mImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapChain.mImageFormat,
                .components = {
                        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .a = VK_COMPONENT_SWIZZLE_IDENTITY
                },
                .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                }
            };

            resultValidation(vkCreateImageView(mDevice, &createInfo, nullptr, &swapChain.mImageViews[i]),
                             "Failed to create image views");
        }
    }

    void Device::destroyImageViews(SwapChain &swapChain) {
        for (auto& imageView : swapChain.mImageViews) {
            vkDestroyImageView(mDevice, imageView, nullptr);
        }
    }

    void Device::createGraphicsPipeline(VkPipeline& graphicsPipeline, VkPipelineLayout& pipelineLayout,
                                        const VkExtent2D& swapChainExtend, const VkRenderPass& renderPass) {
        auto vertexShaderCode = readFile("shaders/shader.vert.spv");
        auto fragmentShaderCode = readFile("shaders/shader.frag.spv");

        VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
        VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShaderModule,
            .pName = "main"
        };

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShaderModule,
            .pName = "main"
        };

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
                vertShaderStageInfo,
                fragShaderStageInfo
        };

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();


        VkPipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        VkViewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapChainExtend.width),
            .height = static_cast<float>(swapChainExtend.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        VkRect2D scissor{
            .offset = { 0, 0 },
            .extent = swapChainExtend
        };

        VkPipelineViewportStateCreateInfo viewportState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor
        };

        VkPipelineRasterizationStateCreateInfo rasterizer{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f
        };

        VkPipelineMultisampleStateCreateInfo multisampling{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        VkPipelineColorBlendAttachmentState colorBlendAttachment{
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                    VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo colorBlending{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
        };

        resultValidation(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout),
                         "Failed to create pipeline layout");

        VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &colorBlending,
            .pDynamicState = nullptr,
            .layout = pipelineLayout,
            .renderPass = renderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        resultValidation(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline),
                         "Failed to create graphics pipeline");

        destroyShaderModule(vertexShaderModule);
        destroyShaderModule(fragmentShaderModule);
    }

    void Device::destroyGraphicsPipeline(VkPipeline& graphicsPipeline, VkPipelineLayout &pipelineLayout) {
        vkDestroyPipeline(mDevice, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(mDevice, pipelineLayout, nullptr);
    }

    VkShaderModule Device::createShaderModule(const std::vector<char> &code) {
        VkShaderModuleCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t*>(code.data())
        };

        VkShaderModule shaderModule;

        resultValidation(vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule),
                         "Failed to create shader module");

        return shaderModule;
    }

    void Device::destroyShaderModule(VkShaderModule &shader) {
        vkDestroyShaderModule(mDevice, shader, nullptr);
    }

    void Device::createRenderPass(VkRenderPass& renderPass, const VkFormat& swapChainFormat) {
        VkAttachmentDescription colorAttachment{
            .format = swapChainFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference colorAttachmentRef{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpass{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef
        };

        VkSubpassDependency dependency{
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        };

        VkRenderPassCreateInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
        };

        resultValidation(vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &renderPass),
                         "Failed to create render pass");
    }

    void Device::destroyRenderPass(VkRenderPass &renderPass) {
        vkDestroyRenderPass(mDevice, renderPass, nullptr);
    }

    void Device::createFramebuffers(SwapChain& swapChain, const VkRenderPass& renderPass) {
        swapChain.mFramebuffers.resize(swapChain.mImageViews.size());

        for (size_t i = 0; i < swapChain.mImageViews.size(); ++i) {
            std::array<VkImageView, 1> attachments = {
                swapChain.mImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = renderPass,
                .attachmentCount = attachments.size(),
                .pAttachments = attachments.data(),
                .width = swapChain.mExtent.width,
                .height = swapChain.mExtent.height,
                .layers = 1
            };

            resultValidation(vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &swapChain.mFramebuffers[i]),
                             "Failed to create framebuffer");
        }
    }

    void Device::destroyFramebuffers(std::vector<VkFramebuffer>& swapChainFramebuffers) {
        for (auto & framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
        }
    }

    void Device::createCommandPool(VkCommandPool &commandPool, VkSurfaceKHR const &surface,
                                   const VkCommandPoolCreateFlags& flags) {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(mPhysicalDevice.device, surface);

        VkCommandPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = flags,
            .queueFamilyIndex = queueFamilyIndices.graphics.value()
        };

        resultValidation(vkCreateCommandPool(mDevice, &poolInfo, nullptr, &commandPool),
                         "Failed to create command pool");
    }

    void Device::destroyCommandPool(VkCommandPool &commandPool) {
        vkDestroyCommandPool(mDevice, commandPool, nullptr);
    }

    void Device::createCommandBuffers(CommandPool& commandPool, const std::vector<VkFramebuffer>& swapChainFramebuffers) {
        commandPool.mBuffers.resize(swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool.mPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(commandPool.mBuffers.size())
        };

        resultValidation(vkAllocateCommandBuffers(mDevice, &allocInfo, commandPool.mBuffers.data()),
                         "Failed to allocate command buffers");
    }

    void Device::freeCommandBuffers(CommandPool& commandPool) {
        vkFreeCommandBuffers(mDevice, commandPool.mPool, static_cast<uint32_t>(commandPool.mBuffers.size()),
                             commandPool.mBuffers.data());
    }

    void Device::createSemaphore(VkSemaphore& semaphore) {
        VkSemaphoreCreateInfo semaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        resultValidation(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &semaphore),
                         "Failed to create semaphores");
    }

    void Device::destroySemaphore(VkSemaphore& semaphore) {
        vkDestroySemaphore(mDevice, semaphore, nullptr);
    }

    VkResult Device::acquireNextImage(uint32_t& imageIndex, const VkSwapchainKHR& swapchain,
                                  const VkSemaphore& imageAvailableSemaphore) {
        return vkAcquireNextImageKHR(mDevice, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore,
                              VK_NULL_HANDLE, &imageIndex);
    }

    void Device::createFence(VkFence& fence) {
        VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        resultValidation(vkCreateFence(mDevice, &fenceInfo, nullptr, &fence),
                         "Failed to create a fence");
    }

    void Device::destroyFence(VkFence &fence) {
        vkDestroyFence(mDevice, fence, nullptr);
    }

    void Device::waitForFence(const VkFence& fence) {
        vkWaitForFences(mDevice, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    }

    void Device::resetFence(VkFence const &fence) {
        vkResetFences(mDevice, 1, &fence);
    }

    void Device::createBuffer(Buffer& buffer, const VkBufferUsageFlags& flags, const VkMemoryPropertyFlags& properties,
                              const VkDeviceSize& size, const VkSharingMode& sharingMode) {
        VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = flags,
            .sharingMode = sharingMode
        };

        resultValidation(vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer.mBuffer),
                         "Failed to create vertex buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(mDevice, buffer.mBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(mPhysicalDevice.device, memRequirements.memoryTypeBits, properties)
        };

        resultValidation(vkAllocateMemory(mDevice, &allocInfo, nullptr, &buffer.mDeviceMemory),
                         "Failed to allocate vertex buffer memory");

        vkBindBufferMemory(mDevice, buffer.mBuffer, buffer.mDeviceMemory, 0);
    }

    void Device::destroyBuffer(Buffer& buffer) {
        vkDestroyBuffer(mDevice, buffer.mBuffer, nullptr);
        vkFreeMemory(mDevice, buffer.mDeviceMemory, nullptr);
    }

    void Device::copyBuffer(VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkCommandPool &commandPool, VkQueue& queue,
                            const VkDeviceSize& size) {
        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

            VkBufferCopy copyRegion{
                .srcOffset = 0,
                .dstOffset = 0,
                .size = size
            };

            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer
        };

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(mDevice, commandPool, 1, &commandBuffer);
    }

} // End namespace vk
