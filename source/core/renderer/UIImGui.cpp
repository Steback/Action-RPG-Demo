#include "UIImGui.hpp"

#include <utility>

#include "../Constants.hpp"
#include "CommandList.hpp"


namespace core {

    UIImGui::UIImGui() = default;

    UIImGui::UIImGui(core::SwapChain &swapChain, const std::shared_ptr<core::Device>& device, GLFWwindow* window,
                     vk::Instance instance, vk::Queue graphicsQueue, std::shared_ptr<CommandList> commandList) {
        m_commands = std::move(commandList);
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontFromFileTTF(std::string(FONTS_DIR + "Roboto-Medium.ttf").c_str(), 16.0f);

        m_device = device->m_logicalDevice;

        createDescriptorPool();
        createRenderPass(static_cast<vk::Format>(swapChain.getFormat()));
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
        vk::CommandBuffer commandBuffer = device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, m_commands->getPool(), true);
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        device->flushCommandBuffer(commandBuffer, graphicsQueue, m_commands->getPool());
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    UIImGui::~UIImGui() = default;

    void UIImGui::cleanup() {
        m_device.destroy(m_descriptorPool);
        m_device.destroy(m_renderPass);
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

    void UIImGui::resize(core::SwapChain& swapChain) {
        ImGui_ImplVulkan_SetMinImageCount(swapChain.getImageCount());
        m_framebuffers.resize(swapChain.getImageCount());
        createFrameBuffers(swapChain);
    }

    void UIImGui::cleanupResources() {
        for (auto& framebuffer : m_framebuffers) {
            m_device.destroy(framebuffer);
        }
    }

    void UIImGui::recordCommands(uint32_t imageIndex, vk::Extent2D swapChainExtent) {
        vk::CommandBuffer& cmdBuffer = m_commands->getBuffer();

        cmdBuffer.begin({
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        });

        vk::ClearValue clearColor = {std::array<float, 4>({{ 0.0f, 0.0f, 0.0f, 1.0f }})};

        cmdBuffer.beginRenderPass({
            .renderPass = m_renderPass,
            .framebuffer = m_framebuffers[imageIndex],
            .renderArea = {
                .extent = swapChainExtent
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor
        }, vk::SubpassContents::eInline);
        {
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_commands->getBuffer());
        }
        cmdBuffer.endRenderPass();

        cmdBuffer.end();
    }

    void UIImGui::createRenderPass(vk::Format swapChainFormat) {
        vk::AttachmentDescription attachmentDescription{
            .format = swapChainFormat,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eLoad,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR
        };

        vk::AttachmentReference attachmentReference{
            .attachment = 0,
            .layout = vk::ImageLayout::eColorAttachmentOptimal
        };

        vk::SubpassDescription subpass{
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentReference
        };

        vk::SubpassDependency subpassDependency{
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                .srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
                .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
        };

        m_renderPass = m_device.createRenderPass({
            .attachmentCount = 1,
            .pAttachments = &attachmentDescription,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &subpassDependency
        });
    }

    void UIImGui::createFrameBuffers(core::SwapChain& swapChain) {
        m_framebuffers.resize(swapChain.getImageCount());

        vk::ImageView attachment[1];
        vk::FramebufferCreateInfo info{
            .renderPass = m_renderPass,
            .attachmentCount = 1,
            .pAttachments = attachment,
            .width = swapChain.getExtent().width,
            .height = swapChain.getExtent().height,
            .layers = 1
        };

        for (uint32_t i = 0; i < swapChain.getImageCount(); ++i) {
            attachment[0] = swapChain.getImageView(i);

            m_framebuffers[i] = m_device.createFramebuffer(info);
        }
    }

    void UIImGui::createDescriptorPool() {
        std::vector<vk::DescriptorPoolSize> poolSizes = {
                { vk::DescriptorType::eSampler, 1000 },
                { vk::DescriptorType::eCombinedImageSampler, 1000 },
                { vk::DescriptorType::eSampledImage, 1000 },
                { vk::DescriptorType::eStorageImage, 1000 },
                { vk::DescriptorType::eUniformTexelBuffer, 1000 },
                { vk::DescriptorType::eStorageTexelBuffer, 1000 },
                { vk::DescriptorType::eUniformBuffer, 1000 },
                { vk::DescriptorType::eStorageBuffer, 1000 },
                { vk::DescriptorType::eUniformBufferDynamic, 1000 },
                { vk::DescriptorType::eStorageBufferDynamic, 1000 },
                { vk::DescriptorType::eInputAttachment, 1000 }
        };

        m_descriptorPool = m_device.createDescriptorPool({
            .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .maxSets = 100 * static_cast<uint32_t>(poolSizes.size()),
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        });
    }

} // namespace core