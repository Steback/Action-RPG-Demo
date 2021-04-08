#include "RenderDevice.hpp"

#include <utility>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "spdlog/spdlog.h"

#include "CommandList.hpp"
#include "../Application.hpp"


inline vk::Format findSupportedFormat(vk::PhysicalDevice device, const std::vector<vk::Format>& candidates,
                             vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
    for (vk::Format format : candidates) {
        vk::FormatProperties props = device.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

inline vk::Result waitForFence(vk::Device device, vk::Fence* fence) {
    return device.waitForFences(1, fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
}

namespace core {

    RenderDevice::RenderDevice(std::shared_ptr<Window> window, vk::Instance instance, const std::string& appName, std::shared_ptr<core::Device> device, vk::SurfaceKHR surface)
            : m_window(std::move(window)), m_device(std::move(device)) {
        m_logicalDevice = m_device->m_logicalDevice;
        m_physicalDevice = m_device->m_physicalDevice;
        m_msaaSamples = m_device->getMaxUsableSampleCount();

        m_graphicsQueue = m_logicalDevice.getQueue(m_device->m_queueFamilyIndices.graphics, 0);

        m_windowSize = m_window->getSize();
        m_swapChain.connect(m_device, surface);
        m_swapChain.create(m_windowSize.width, m_windowSize.height, m_device->m_queueFamilyIndices.graphics, m_device->m_queueFamilyIndices.graphics);

        m_depthFormat= findSupportedFormat(m_physicalDevice,
                                           {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                           vk::ImageTiling::eOptimal,
                                           vk::FormatFeatureFlagBits::eDepthStencilAttachment);

        m_ui = core::UIImGui(m_swapChain, m_device, m_window->getWindow(), instance, m_graphicsQueue,
                             addCommandList());
    }

    RenderDevice::~RenderDevice() = default;

    void RenderDevice::init(bool drawGrid) {
        m_drawGrid = drawGrid;

        m_pipeline = std::make_unique<core::GraphicsPipeline>(core::Application::m_resourceManager->createShader("shaders/model.vert.spv", "shaders/model.frag.spv"), m_device->m_logicalDevice);

        if (drawGrid)
            m_gridPipeline = std::make_unique<core::GraphicsPipeline>(core::Application::m_resourceManager->createShader("shaders/grid.vert.spv", "shaders/grid.frag.spv", false), m_device->m_logicalDevice);

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

    void RenderDevice::cleanup(const std::shared_ptr<core::Instance>& instance) {
        UIImGui::cleanupImGui();

        cleanSwapChain();

        m_ui.cleanup();
        instance->destroy(m_swapChain.getSurface());

        m_logicalDevice.destroy(m_descriptorSetLayout);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            m_logicalDevice.destroy(m_imageAvailableSemaphores[i]);
            m_logicalDevice.destroy(m_renderFinishedSemaphores[i]);
            m_logicalDevice.destroy(m_fences[i]);
        }

        for (auto& cmdList : m_commands) {
            cmdList->cleanup();
        }

        spdlog::info("[Renderer] Cleaned");
    }

    void RenderDevice::acquireNextImage() {
        VK_CHECK_RESULT_HPP(waitForFence(m_logicalDevice, &m_fences[m_currentFrame]))

        vk::Result result = m_swapChain.acquireNextImage(m_imageAvailableSemaphores[m_currentFrame], &m_indexImage);

        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapchain();
            return;
        } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        if (m_imageFences[m_indexImage])
        VK_CHECK_RESULT_HPP(waitForFence(m_logicalDevice, &m_imageFences[m_indexImage]))

        m_imageFences[m_indexImage] = m_fences[m_currentFrame];
    }

    void RenderDevice::render() {
        m_ui.recordCommands(m_indexImage, m_swapChain.getExtent());

        VK_CHECK_RESULT_HPP(m_logicalDevice.resetFences(1, &m_fences[m_currentFrame]))

        vk::Semaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::Semaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };

        // TODO: Check to dynamic add command buffer to summit queue
        std::vector<vk::CommandBuffer> cmdBuffers{
            m_commands[1]->getBuffer(),
            m_commands[0]->getBuffer()
        };

        m_graphicsQueue.submit(vk::SubmitInfo{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = static_cast<uint32_t>(cmdBuffers.size()),
            .pCommandBuffers = cmdBuffers.data(),
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores
        }, m_fences[m_currentFrame]);

        vk::SwapchainKHR swapChains[] = { m_swapChain.getSwapChain() };
        vk::Result result = m_graphicsQueue.presentKHR({
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = &m_indexImage,
            .pResults = nullptr
        });

        // TODO: Check error in resize window
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_window->resize()) {
            m_window->resize() = false;
            recreateSwapchain();
        } else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to present swap chain image");
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void RenderDevice::createRenderPass() {
        vk::AttachmentDescription colorAttachment{
            .format = m_swapChain.getFormat(),
            .samples = m_msaaSamples,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eColorAttachmentOptimal
        };

        vk::AttachmentReference colorAttachmentRef{
            .attachment = 0,
            .layout = vk::ImageLayout::eColorAttachmentOptimal
        };

        vk::AttachmentDescription depthAttachment{
            .format = m_depthFormat,
            .samples = m_msaaSamples,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eDontCare,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal
        };

        vk::AttachmentReference depthAttachmentRef{
            .attachment = 1,
            .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal
        };

        vk::AttachmentDescription colorAttachmentResolve{
            .format = m_swapChain.getFormat(),
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eDontCare,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eColorAttachmentOptimal
        };

        vk::AttachmentReference colorAttachmentResolveRef{
            .attachment = 2,
            .layout = vk::ImageLayout::eColorAttachmentOptimal
        };

        vk::SubpassDescription subpass{
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pResolveAttachments = &colorAttachmentResolveRef,
            .pDepthStencilAttachment = &depthAttachmentRef,
        };

        vk::SubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = {},
            .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
            .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
            .srcAccessMask = {},
            .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
        };

        std::array<vk::AttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
        m_renderPass = m_logicalDevice.createRenderPass({
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
        });
    }

    void RenderDevice::createGraphicsPipeline() {
        std::vector<vk::DescriptorSetLayout> layouts = {
                m_descriptorSetLayout,
                core::Application::m_resourceManager->getTextureDescriptorSetLayout()
        };

        m_pipeline->create({m_mvpRange}, layouts, m_swapChain, m_renderPass, m_msaaSamples);

        if (m_drawGrid) createGridPipeline();
    }

    void RenderDevice::createFramebuffers() {
        m_framebuffers.resize(m_swapChain.getImageCount());

        for (size_t i = 0; i < m_swapChain.getImageCount(); ++i) {
            std::array<vk::ImageView, 3> attachments = {
                    m_colorImage.getView(),
                    m_depthBuffer.getView(),
                    m_swapChain.getImageView(i)
            };

            m_framebuffers[i] = m_logicalDevice.createFramebuffer({
                .renderPass = m_renderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = m_swapChain.getExtent().width,
                .height = m_swapChain.getExtent().height,
                .layers = 1
            });
        }
    }

    void RenderDevice::createSyncObjects() {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_fences.resize(MAX_FRAMES_IN_FLIGHT);
        m_imageFences.resize(m_swapChain.getImageCount(), nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            m_imageAvailableSemaphores[i] = m_logicalDevice.createSemaphore({});
            m_renderFinishedSemaphores[i] = m_logicalDevice.createSemaphore({});

            m_fences[i] = m_logicalDevice.createFence({
                .flags = vk::FenceCreateFlagBits::eSignaled
            });
        }
    }

    void RenderDevice::recreateSwapchain() {
        // TODO: Optimize window resize
        while (m_windowSize.width == 0 || m_windowSize.height == 0)  {
            m_windowSize = m_window->getSize();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_logicalDevice);

        cleanSwapChain();

        m_swapChain.create(m_windowSize.width, m_windowSize.height, m_device->m_queueFamilyIndices.graphics, m_device->m_queueFamilyIndices.graphics);
        createRenderPass();
        createGraphicsPipeline();
        createMsaaResources();
        createDepthResources();
        createFramebuffers();
        createDescriptorPool();
        createDescriptorSets();

        for (auto& cmd : m_commands) cmd->initBuffers(m_swapChain.getImageCount(), &m_indexImage);

        m_ui.resize(m_swapChain);
        core::Application::m_resourceManager->recreateResources();
    }

    void RenderDevice::cleanSwapChain() {
        m_depthBuffer.cleanup(m_logicalDevice);
        m_colorImage.cleanup(m_logicalDevice);

        m_ui.cleanupResources();

        for (auto & framebuffer : m_framebuffers) m_logicalDevice.destroy(framebuffer);

        for (auto& cmd : m_commands) cmd->free();

        if (m_drawGrid) m_gridPipeline->cleanup();

        m_pipeline->cleanup();
        m_logicalDevice.destroy(m_renderPass);

        m_swapChain.cleanup();

        m_logicalDevice.destroy(m_descriptorPool);
        core::Application::m_resourceManager->cleanupResources();
    }

    void RenderDevice:: createDescriptorSetLayout() {
        vk::DescriptorSetLayoutBinding uboLayoutBinding{
            .binding = 0,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .pImmutableSamplers = nullptr
        };

        m_descriptorSetLayout = m_logicalDevice.createDescriptorSetLayout({
            .bindingCount = 1,
            .pBindings = &uboLayoutBinding
        });
    }

    void RenderDevice::createDescriptorPool() {
        vk::DescriptorPoolSize descriptorPoolSize{
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = m_swapChain.getImageCount()
        };

        m_descriptorPool = m_logicalDevice.createDescriptorPool({
            .flags = {},
            .maxSets = m_swapChain.getImageCount(),
            .poolSizeCount = 1,
            .pPoolSizes = &descriptorPoolSize,
        });
    }

    void RenderDevice::createDescriptorSets() {
        std::vector<vk::DescriptorSetLayout> layouts(m_swapChain.getImageCount(), m_descriptorSetLayout);

        m_descriptorSets = m_logicalDevice.allocateDescriptorSets({
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = m_swapChain.getImageCount(),
            .pSetLayouts = layouts.data()
        });
    }

    void RenderDevice::createDepthResources() {
        m_depthBuffer = core::Image(m_logicalDevice, {
                .imageType = vk::ImageType::e2D,
                .format = static_cast<vk::Format>(m_depthFormat),
                .extent = {
                        .width = m_swapChain.getExtent().width,
                        .height = m_swapChain.getExtent().height,
                        .depth = 1
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = static_cast<vk::SampleCountFlagBits>(m_msaaSamples),
                .tiling = vk::ImageTiling::eOptimal,
                .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
                .sharingMode = vk::SharingMode::eExclusive,
                .initialLayout = vk::ImageLayout::eUndefined
        });

        vk::MemoryRequirements memoryRequirements = m_logicalDevice.getImageMemoryRequirements(m_depthBuffer.getImage());
        uint32_t memType = m_device->getMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

        m_depthBuffer.bind(m_logicalDevice, memType, memoryRequirements.size, vk::ImageAspectFlagBits::eDepth);

        m_device->transitionImageLayout(m_depthBuffer.getImage(), static_cast<vk::Format>(m_depthBuffer.getFormat()), m_graphicsQueue,
                                        vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    }

    void RenderDevice::createMsaaResources() {
        auto colorFormat = static_cast<vk::Format>(m_swapChain.getFormat());

        m_colorImage = core::Image(m_logicalDevice, {
                .imageType = vk::ImageType::e2D,
                .format = static_cast<vk::Format>(colorFormat),
                .extent = {
                        .width = m_swapChain.getExtent().width,
                        .height = m_swapChain.getExtent().height,
                        .depth = 1
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = static_cast<vk::SampleCountFlagBits>(m_msaaSamples),
                .tiling = vk::ImageTiling::eOptimal,
                .usage = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
                .sharingMode = vk::SharingMode::eExclusive,
                .initialLayout = vk::ImageLayout::eUndefined
        });

        vk::MemoryRequirements memoryRequirements = m_logicalDevice.getImageMemoryRequirements(m_colorImage.getImage());
        uint32_t  memType = m_device->getMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

        m_colorImage.bind(m_logicalDevice, memType, memoryRequirements.size, vk::ImageAspectFlagBits::eColor);
    }

    void RenderDevice::createPushConstants() {
        m_mvpRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
        m_mvpRange.offset = 0;
        m_mvpRange.size = sizeof(MVP);
    }

    void RenderDevice::createGridPipeline() {
        std::vector<vk::DescriptorSetLayout> layouts = {
                m_descriptorSetLayout,
                core::Application::m_resourceManager->getTextureDescriptorSetLayout()
        };

        m_gridPipeline->create({m_mvpRange}, layouts, m_swapChain, m_renderPass, m_msaaSamples);
    }

    void RenderDevice::updateVP(const glm::mat4& view, const glm::mat4& proj) {
        m_mvp.view = view;
        m_mvp.proj = proj;
    }

    vk::Queue &RenderDevice::getGraphicsQueue() {
        return m_graphicsQueue;
    }

    std::shared_ptr<CommandList> RenderDevice::addCommandList() {
        m_commands.push_back(std::make_shared<CommandList>(m_device->createCommandPool(), m_logicalDevice));
        std::shared_ptr<CommandList> cmd = m_commands.back();

        cmd->initBuffers(m_swapChain.getImageCount(), &m_indexImage);

        return cmd;
    }

    vk::Framebuffer &RenderDevice::getFrameBuffer() {
        return m_framebuffers[m_indexImage];
    }

    vk::Extent2D RenderDevice::getSwapChainExtent() {
        return m_swapChain.getExtent();
    }

    vk::RenderPass &RenderDevice::getRenderPass() {
        return m_renderPass;
    }

    vk::DescriptorSet &RenderDevice::getDescriptorSet() {
        return m_descriptorSets[m_indexImage];
    }

} // End namespace core