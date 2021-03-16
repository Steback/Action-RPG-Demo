#include "UIImGui.hpp"

#include "Tools.hpp"
#include "../Constants.hpp"


namespace core {

    UIImGui::UIImGui() = default;

    UIImGui::UIImGui(vk::SwapChain &swapChain, vk::Device *device, GLFWwindow* window, VkInstance instance, VkQueue graphicsQueue) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontFromFileTTF(std::string(FONTS_DIR + "Roboto-Medium.ttf").c_str(), 16.0f);

        m_device = device;

        createDescriptorPool();
        createRenderPass(swapChain.getFormat());
        createCommandPool();
        createCommandBuffers(swapChain.getImageCount());
        createFrameBuffers(swapChain);

        // Provide bind points from Vulkan API
        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance;
        init_info.PhysicalDevice = device->m_physicalDevice;
        init_info.Device = device->m_logicalDevice;
        init_info.QueueFamily = device->m_queueFamilyIndices.graphics;
        init_info.Queue = graphicsQueue;
        init_info.PipelineCache = nullptr;
        init_info.DescriptorPool = m_descriptorPool;
        init_info.MinImageCount = swapChain.getImageCount();
        init_info.ImageCount = swapChain.getImageCount();
        ImGui_ImplVulkan_Init(&init_info, m_renderPass);

        // Upload the fonts for Dear ImGui
        VkCommandBuffer commandBuffer = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool, true);
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        device->flushCommandBuffer(commandBuffer, graphicsQueue, m_commandPool, true);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    UIImGui::~UIImGui() = default;

    void UIImGui::cleanup() {
        vkDestroyDescriptorPool(m_device->m_logicalDevice, m_descriptorPool, nullptr);
        vkDestroyCommandPool(m_device->m_logicalDevice, m_commandPool, nullptr);
        vkDestroyRenderPass(m_device->m_logicalDevice, m_renderPass, nullptr);
    }

    void UIImGui::cleanupImGui() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void UIImGui::newFrame() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void UIImGui::render() {
        ImGui::Render();
    }

    void UIImGui::resize(vk::SwapChain& swapChain) {
        ImGui_ImplVulkan_SetMinImageCount(swapChain.getImageCount());
        m_framebuffers.resize(swapChain.getImageCount());
        createFrameBuffers(swapChain);
        createCommandBuffers(swapChain.getImageCount());
    }

    void UIImGui::cleanupResources() {
        for (auto& framebuffer : m_framebuffers) {
            vkDestroyFramebuffer(m_device->m_logicalDevice, framebuffer, nullptr);
        }

        vkFreeCommandBuffers(m_device->m_logicalDevice, m_commandPool, static_cast<uint64_t>(m_commandBuffers.size()),
                              m_commandBuffers.data());
    }

    VkCommandBuffer UIImGui::getCommandBuffer(uint32_t imageIndex) {
        return m_commandBuffers[imageIndex];
    }

    void UIImGui::recordCommands(uint32_t imageIndex, VkExtent2D swapChainExtent) {
        VkCommandBufferBeginInfo cmdBufferBegin = vk::initializers::commandBufferBeginInfo();
        cmdBufferBegin.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK_RESULT(vkBeginCommandBuffer(m_commandBuffers[imageIndex], &cmdBufferBegin));

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

        VkRenderPassBeginInfo renderPassBeginInfo = vk::initializers::renderPassBeginInfo();
        renderPassBeginInfo.renderPass = m_renderPass;
        renderPassBeginInfo.framebuffer = m_framebuffers[imageIndex];
        renderPassBeginInfo.renderArea.extent.width = swapChainExtent.width;
        renderPassBeginInfo.renderArea.extent.height = swapChainExtent.height;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            // Grab and record the transform data for Dear Imgui
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_commandBuffers[imageIndex]);
        }
        // End and submit render pass
        vkCmdEndRenderPass(m_commandBuffers[imageIndex]);

        VK_CHECK_RESULT(vkEndCommandBuffer(m_commandBuffers[imageIndex]));
    }

    void UIImGui::createRenderPass(VkFormat swapChainFormat) {
        VkAttachmentDescription attachmentDescription = {};
        attachmentDescription.format = swapChainFormat;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        VkAttachmentReference attachmentReference = {};
        attachmentReference.attachment = 0;
        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentReference;

        VkSubpassDependency subpassDependency = {};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = vk::initializers::renderPassCreateInfo();
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachmentDescription;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &subpassDependency;

        VK_CHECK_RESULT(vkCreateRenderPass(m_device->m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass));
    }

    void UIImGui::createFrameBuffers(vk::SwapChain& swapChain) {
        m_framebuffers.resize(swapChain.getImageCount());

        VkImageView attachment[1];
        VkFramebufferCreateInfo info = vk::initializers::framebufferCreateInfo();
        info.renderPass = m_renderPass;
        info.attachmentCount = 1;
        info.pAttachments = attachment;
        info.width = swapChain.getExtent().width;
        info.height = swapChain.getExtent().height;
        info.layers = 1;

        for (uint32_t i = 0; i < swapChain.getImageCount(); ++i) {
            attachment[0] = swapChain.getImageView(i);

            VK_CHECK_RESULT(vkCreateFramebuffer(m_device->m_logicalDevice, &info, nullptr, &m_framebuffers[i]));
        }
    }

    void UIImGui::createCommandPool() {
        m_commandPool = m_device->createCommandPool(m_device->m_queueFamilyIndices.graphics);
    }

    void UIImGui::createCommandBuffers(uint32_t imageCount) {
        m_commandBuffers.resize(imageCount);

        for (int i = 0; i < imageCount; ++i) {
            m_commandBuffers[i] = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool);
        }
    }

    void UIImGui::createDescriptorPool() {
        VkDescriptorPoolSize pool_sizes[] = {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * std::size(pool_sizes);
        pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
        pool_info.pPoolSizes = pool_sizes;

        VK_CHECK_RESULT(vkCreateDescriptorPool(m_device->m_logicalDevice, &pool_info, nullptr, &m_descriptorPool));
    }

} // namespace core