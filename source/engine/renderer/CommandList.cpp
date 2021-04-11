#include "CommandList.hpp"

#include "../Utilities.hpp"


namespace engine {


    CommandList::CommandList(vk::CommandPool m_pool, vk::Device device, bool sharedPool)
            : m_pool(m_pool), m_device(device), m_sharedPool(sharedPool) {

    }

    void CommandList::cleanup() {
        if (!m_sharedPool) m_device.destroy(m_pool);
    }

    void CommandList::initBuffers(uint32_t imageCount, uint32_t *imageIndex) {
        m_imageIndex = imageIndex;
        m_buffers.resize(imageCount);

        m_buffers = m_device.allocateCommandBuffers({
            .commandPool = m_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = imageCount,
        });
    }

    vk::CommandBuffer &CommandList::getBuffer() {
        uint32_t index = (m_imageIndex ? *m_imageIndex : 0);

        return m_buffers[index];
    }

    vk::CommandPool &CommandList::getPool() {
        return m_pool;
    }

    void CommandList::free() {
        m_device.free(m_pool, m_buffers);
    }

    void CommandList::beginRenderPass(const vk::RenderPass &renderPass, const glm::vec4& clearColor, vk::Framebuffer& framebuffer,
                                      vk::Extent2D swapChainExtent) {
        std::array<vk::ClearValue, 2> clearValues;
        clearValues[0].color = {std::array<float, 4>({{clearColor.x, clearColor.y, clearColor.z, clearColor.w}})};
        clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

        m_buffers[*m_imageIndex].beginRenderPass({
            .renderPass = renderPass,
            .framebuffer = framebuffer,
            .renderArea = {
                    .offset = vk::Offset2D{0, 0},
                    .extent = swapChainExtent,
            },
                    .clearValueCount = static_cast<uint32_t>(clearValues.size()),
                    .pClearValues = clearValues.data()
        }, vk::SubpassContents::eInline);
    }

    void CommandList::endRenderPass() {
        m_buffers[*m_imageIndex].endRenderPass();
    }

    void CommandList::begin(vk::CommandBufferUsageFlagBits usage) {
        m_buffers[*m_imageIndex].begin({
            .flags = usage
        });
    }

    void CommandList::end() {
        m_buffers[*m_imageIndex].end();
    }

} // namespace core