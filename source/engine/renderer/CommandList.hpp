#ifndef PROTOTYPE_ACTION_RPG_COMMANDLIST_HPP
#define PROTOTYPE_ACTION_RPG_COMMANDLIST_HPP


#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"


namespace engine {
    class CommandList {
    public:
        explicit CommandList(vk::CommandPool m_pool, vk::Device device, bool sharedPool = false);

        void cleanup();

        void initBuffers(uint32_t imageCount = 1, uint32_t* imageIndex = nullptr);

        void free();

        vk::CommandBuffer& getBuffer();

        vk::CommandPool& getPool();

        void beginRenderPass(const vk::RenderPass& renderPass, const glm::vec4& clearColor, vk::Framebuffer& framebuffer,
                             vk::Extent2D swapChainExtent);

        void endRenderPass();

        void begin(vk::CommandBufferUsageFlagBits usage = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        void end();

    private:
        uint32_t* m_imageIndex{};
        std::vector<vk::CommandBuffer> m_buffers;
        vk::CommandPool m_pool;
        bool m_sharedPool;
        vk::Device m_device;
    };
} // namespace core


#endif //PROTOTYPE_ACTION_RPG_COMMANDLIST_HPP
