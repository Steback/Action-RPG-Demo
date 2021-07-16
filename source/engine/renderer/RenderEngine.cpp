#include "RenderEngine.hpp"

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

namespace engine {

    RenderEngine::RenderEngine(std::shared_ptr<Window> window, vk::Instance instance, const std::string& appName, std::shared_ptr<engine::Device> device, vk::SurfaceKHR surface)
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
    }

    RenderEngine::~RenderEngine() = default;

    void RenderEngine::init() {
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createMsaaResources();
        createDepthResources();
        createFramebuffers();
        createDescriptorPool();
        createDescriptorSets();
        createSyncObjects();

        spdlog::info("[Renderer] Initialized");
    }

    void RenderEngine::cleanup(const std::shared_ptr<engine::Instance>& instance) {
        UIRender::cleanupImGui();

        cleanSwapChain();

        instance->destroy(m_swapChain.getSurface());

        m_logicalDevice.destroy(m_descriptorSetLayout);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            m_logicalDevice.destroy(m_imageAvailableSemaphores[i]);
            m_logicalDevice.destroy(m_renderFinishedSemaphores[i]);
            m_logicalDevice.destroy(m_fences[i]);
        }

        for (auto& cmdList : m_mainCommands) {
            cmdList->cleanup();
        }

        spdlog::info("[Renderer] Cleaned");
    }

    void RenderEngine::acquireNextImage() {
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

    void RenderEngine::render() {
        VK_CHECK_RESULT_HPP(m_logicalDevice.resetFences(1, &m_fences[m_currentFrame]))

        vk::Semaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::Semaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };

        std::vector<vk::CommandBuffer> cmdBuffers(m_mainCommands.size());

        for (int i = 0; i < m_mainCommands.size(); ++i) cmdBuffers[i] = m_mainCommands[i]->getBuffer();

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            m_graphicsQueue.submit(vk::SubmitInfo{
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = waitSemaphores,
                    .pWaitDstStageMask = waitStages,
                    .commandBufferCount = static_cast<uint32_t>(cmdBuffers.size()),
                    .pCommandBuffers = cmdBuffers.data(),
                    .signalSemaphoreCount = 1,
                    .pSignalSemaphores = signalSemaphores
            }, m_fences[m_currentFrame]);
        }

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

    void RenderEngine::createRenderPass() {
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

    void RenderEngine::createGraphicsPipeline() {
        std::vector<vk::DescriptorSetLayout> layouts = {
                m_descriptorSetLayout,
                engine::Application::m_resourceManager->getTextureDescriptorSetLayout()
        };

        for (auto& pipeline : m_pipelines)
            pipeline->create(layouts, m_swapChain, m_renderPass, m_msaaSamples);
    }

    void RenderEngine::createFramebuffers() {
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

    void RenderEngine::createSyncObjects() {
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

    void RenderEngine::recreateSwapchain() {
        // TODO: Optimize window resize
        while (m_windowSize.width == 0 || m_windowSize.height == 0)  {
            m_windowSize = m_window->getSize();
            glfwWaitEvents();
        }

        m_logicalDevice.waitIdle();

        cleanSwapChain();

        m_swapChain.create(m_windowSize.width, m_windowSize.height, m_device->m_queueFamilyIndices.graphics, m_device->m_queueFamilyIndices.graphics);
        createRenderPass();
        createGraphicsPipeline();
        createMsaaResources();
        createDepthResources();
        createFramebuffers();
        createDescriptorPool();
        createDescriptorSets();

        for (auto& cmd : m_mainCommands) cmd->initBuffers(m_swapChain.getImageCount(), &m_indexImage);

        // TODO: Include UIImGui in resize window
//        m_ui.resize(m_swapChain);
        engine::Application::m_resourceManager->recreateResources();
    }

    void RenderEngine::cleanSwapChain() {
        m_depthBuffer.cleanup(m_logicalDevice);
        m_colorImage.cleanup(m_logicalDevice);

        for (auto & framebuffer : m_framebuffers) m_logicalDevice.destroy(framebuffer);

        for (auto& cmd : m_mainCommands) cmd->free();

        for (auto& pipeline : m_pipelines) pipeline->cleanup();

        m_logicalDevice.destroy(m_renderPass);

        m_swapChain.cleanup();

        m_logicalDevice.destroy(m_descriptorPool);
        engine::Application::m_resourceManager->cleanupResources();
    }

    void RenderEngine:: createDescriptorSetLayout() {
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

    void RenderEngine::createDescriptorPool() {
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

    void RenderEngine::createDescriptorSets() {
        std::vector<vk::DescriptorSetLayout> layouts(m_swapChain.getImageCount(), m_descriptorSetLayout);

        m_descriptorSets = m_logicalDevice.allocateDescriptorSets({
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = m_swapChain.getImageCount(),
            .pSetLayouts = layouts.data()
        });
    }

    void RenderEngine::createDepthResources() {
        m_depthBuffer = engine::Image(m_logicalDevice, {
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

    void RenderEngine::createMsaaResources() {
        auto colorFormat = static_cast<vk::Format>(m_swapChain.getFormat());

        m_colorImage = engine::Image(m_logicalDevice, {
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

    void RenderEngine::updateVP(const glm::mat4& view, const glm::mat4& proj) {
        m_mvp.view = view;
        m_mvp.proj = proj;
    }

    vk::Queue &RenderEngine::getGraphicsQueue() {
        return m_graphicsQueue;
    }

    std::shared_ptr<CommandList> RenderEngine::addCommandList() {
        m_mainCommands.push_back(std::make_shared<CommandList>(m_device->createCommandPool(), m_logicalDevice));
        std::shared_ptr<CommandList> cmd = m_mainCommands.back();

        cmd->initBuffers(m_swapChain.getImageCount(), &m_indexImage);

        return cmd;
    }

    vk::Framebuffer &RenderEngine::getFrameBuffer() {
        return m_framebuffers[m_indexImage];
    }

    vk::Extent2D RenderEngine::getSwapChainExtent() {
        return m_swapChain.getExtent();
    }

    vk::RenderPass &RenderEngine::getRenderPass() {
        return m_renderPass;
    }

    vk::DescriptorSet &RenderEngine::getDescriptorSet() {
        return m_descriptorSets[m_indexImage];
    }

    std::shared_ptr<GraphicsPipeline> RenderEngine::addPipeline(const std::shared_ptr<engine::Shader>& shaderID, vk::Device device,
                                                                std::vector<vk::DescriptorSetLayout>* pLayouts, bool inited) {
        m_pipelines.push_back(std::make_shared<GraphicsPipeline>(shaderID, device));

        if (inited) {
            if (pLayouts) {
                m_pipelines.back()->create(*pLayouts, m_swapChain, m_renderPass, m_msaaSamples);
            } else {
                std::vector<vk::DescriptorSetLayout> layouts = {
                        m_descriptorSetLayout,
                        engine::Application::m_resourceManager->getTextureDescriptorSetLayout()
                };

                m_pipelines.back()->create(layouts, m_swapChain, m_renderPass, m_msaaSamples);
            }
        }

        return m_pipelines.back();
    }

    SwapChain &RenderEngine::getSwapChain() {
        return m_swapChain;
    }

    uint32_t RenderEngine::getImageIndex() const {
        return m_indexImage;
    }

    vk::DescriptorSetLayout RenderEngine::getDescriptorSetLayout() {
        return m_descriptorSetLayout;
    }

} // End namespace core