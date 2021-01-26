#include "Renderer.hpp"

#include "Constants.hpp"
#include "../mesh/Mesh.hpp"


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
        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Prototype Action RPG",
            .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
            .pEngineName = "Custom Engine",
            .engineVersion = VK_MAKE_VERSION(0, 1, 0),
            .apiVersion = VK_API_VERSION_1_2
        };

        m_instance.init(appInfo);
        m_instance.createSurface(m_window->m_window, m_surface);
        m_instance.pickPhysicalDevice(m_physicialDevice, m_surface);
        m_device.init(m_physicialDevice, m_surface, m_graphicsQueue, m_presentQueue);
        m_device.createSwapChain(m_swapchain, m_window->getSize(), m_surface);
        m_device.createImageViews(m_swapchain);
        m_device.createRenderPass(m_renderPass, m_swapchain.format);
        m_device.createGraphicsPipeline(m_graphicsPipeline, m_pipelineLayout, m_swapchain.extent, m_renderPass);
        m_device.createFramebuffers(m_swapchain, m_renderPass);
        m_device.createCommandPool(m_commandPool.mPool, m_surface);
        m_device.createCommandPool(m_transferCommandPool, m_surface, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
        createVertexBuffer();
        createIndexBuffer();
        m_device.createCommandBuffers(m_commandPool, m_swapchain.framebuffers);
        recordCommands();

        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_fences.resize(MAX_FRAMES_IN_FLIGHT);
        m_imageFences.resize(m_swapchain.imageViews.size(), VK_NULL_HANDLE);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            m_device.createSemaphore(m_imageAvailableSemaphores[i]);
            m_device.createSemaphore(m_renderFinishedSemaphores[i]);
            m_device.createFence(m_fences[i]);
        }

        spdlog::info("[Renderer] Initialized");
    }

    Renderer::~Renderer() = default;

    void Renderer::draw() {
        m_device.waitForFence(m_fences[m_currentFrame]);

        uint32_t indexImage;
        VkResult result = m_device.acquireNextImage(indexImage, m_swapchain.swapchain,
                                                    m_imageAvailableSemaphores[m_currentFrame]);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapchain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            spdlog::throw_spdlog_ex("Failed to acquire swap chain image");
        }

        if (m_imageFences[indexImage] != VK_NULL_HANDLE) {
            m_device.waitForFence(m_imageFences[indexImage]);
        }

        m_imageFences[indexImage] = m_fences[m_currentFrame];

        VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame] };

        VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphores,
                .pWaitDstStageMask = waitStages,
                .commandBufferCount = 1,
                .pCommandBuffers = &m_commandPool.mBuffers[indexImage],
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = signalSemaphores
        };

        m_device.resetFence(m_fences[m_currentFrame]);

        vk::resultValidation(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_fences[m_currentFrame]),
                             "Failed to submit draw command buffer");

        VkSwapchainKHR swapChains[] = {m_swapchain.swapchain };

        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = &indexImage,
            .pResults = nullptr
        };

        result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->m_resize) {
            m_window->m_resize = false;
            recreateSwapchain();
        } else if (result != VK_SUCCESS) {
            spdlog::throw_spdlog_ex("Failed to present swap chain image");
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::clean() {
        m_device.waitIdle();

        cleanSwapChain();

        m_device.destroyBuffer(m_indexBuffer);
        m_device.destroyBuffer(m_vertexBuffer);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            m_device.destroySemaphore(m_imageAvailableSemaphores[i]);
            m_device.destroySemaphore(m_renderFinishedSemaphores[i]);
            m_device.destroyFence(m_fences[i]);
        }

        m_device.destroyCommandPool(m_commandPool.mPool);
        m_device.destroyCommandPool(m_transferCommandPool);
        m_device.destroy();
        m_instance.destroySurface(m_surface);
        m_instance.destroy();

        spdlog::info("[Renderer] Cleaned");
    }

    void Renderer::recordCommands() {
        VkClearValue clearColors[] = { {0.0f, 0.0f, 0.0f, 1.0f} };

        for (size_t i = 0; i < m_commandPool.mBuffers.size(); ++i) {
            VkCommandBufferBeginInfo beginInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .pInheritanceInfo = nullptr
            };

            vk::resultValidation(vkBeginCommandBuffer(m_commandPool.mBuffers[i], &beginInfo),
                             "Failed to begin recording command buffer");

            VkRenderPassBeginInfo renderPassInfo{
                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                    .renderPass = m_renderPass,
                    .framebuffer = m_swapchain.framebuffers[i],
                    .renderArea = {
                            .offset = { 0, 0 },
                            .extent = m_swapchain.extent
                    },
                    .clearValueCount = 1,
                    .pClearValues = clearColors
            };

                vkCmdBeginRenderPass(m_commandPool.mBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    vkCmdBindPipeline(m_commandPool.mBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

                    VkBuffer vertexBuffer[] = {m_vertexBuffer.buffer };
                    VkDeviceSize offsets[] = { 0 };
                    vkCmdBindVertexBuffers(m_commandPool.mBuffers[i], 0, 1, vertexBuffer, offsets);
                    vkCmdBindIndexBuffer(m_commandPool.mBuffers[i], m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

                    vkCmdDrawIndexed(m_commandPool.mBuffers[i], static_cast<uint32_t>(indices.size()),
                                     1, 0, 0, 0);

                vkCmdEndRenderPass(m_commandPool.mBuffers[i]);

            vk::resultValidation(vkEndCommandBuffer(m_commandPool.mBuffers[i]),
                                 "Failed to record command buffer");
        }
    }

    void Renderer::recreateSwapchain() {
        // TODO: Optimize window resize
        while (m_window->m_size.width == 0 || m_window->m_size.height == 0)  {
            glfwWaitEvents();
        }

        m_device.waitIdle();

        cleanSwapChain();

        m_device.createSwapChain(m_swapchain, m_window->getSize(), m_surface, true);
        m_device.createImageViews(m_swapchain);
        m_device.createRenderPass(m_renderPass, m_swapchain.format);
        m_device.createGraphicsPipeline(m_graphicsPipeline, m_pipelineLayout, m_swapchain.extent, m_renderPass);
        m_device.createFramebuffers(m_swapchain, m_renderPass);
        m_device.createCommandBuffers(m_commandPool, m_swapchain.framebuffers);
        recordCommands();
    }

    void Renderer::cleanSwapChain() {
        m_device.destroyFramebuffers(m_swapchain.framebuffers);
        m_device.freeCommandBuffers(m_commandPool);
        m_device.destroyGraphicsPipeline(m_graphicsPipeline, m_pipelineLayout);
        m_device.destroyRenderPass(m_renderPass);
        m_device.destroyImageViews(m_swapchain);
        m_device.destroySwapChain(m_swapchain);
    }

    void Renderer::createVertexBuffer() {
        VkDeviceSize size = sizeof(Vertex) * vertices.size();
        vk::Buffer stagingBuffer;

        m_device.createBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size);

        m_device.copyData(stagingBuffer, vertices.data(), size);

        m_device.createBuffer(m_vertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, size);

        m_device.copyBuffer(stagingBuffer.buffer, m_vertexBuffer.buffer, m_transferCommandPool,
                            m_graphicsQueue, size);

        m_device.destroyBuffer(stagingBuffer);
    }

    void Renderer::createIndexBuffer() {
        VkDeviceSize size = sizeof(uint16_t) * indices.size();
        vk::Buffer stagingBuffer;

        m_device.createBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size);

        m_device.copyData(stagingBuffer, indices.data(), size);

        m_device.createBuffer(m_indexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, size);

        m_device.copyBuffer(stagingBuffer.buffer, m_indexBuffer.buffer, m_transferCommandPool,
                            m_graphicsQueue, size);

        m_device.destroyBuffer(stagingBuffer);
    }

} // End namespace core