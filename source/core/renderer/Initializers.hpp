#ifndef PROTOTYPE_ACTION_RPG_INITIALIZERS_HPP
#define PROTOTYPE_ACTION_RPG_INITIALIZERS_HPP


#include "vulkan/vulkan.h"


namespace vk::initializers {

    inline VkApplicationInfo applicationInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO
        };
    }

    inline VkDeviceQueueCreateInfo deviceQueueCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO
        };
    }

    inline VkDeviceCreateInfo deviceCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO
        };
    }

    inline VkSwapchainCreateInfoKHR swapchainCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR
        };
    }

    inline VkImageViewCreateInfo imageViewCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO
        };
    }

    inline VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
        };
    }

    inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
        };
    }

    inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
        };
    }

    inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
        };
    }

    inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
        };
    }

    inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
        };
    }

    inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
        };
    }

    inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
        };
    }

    inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
        };
    }

    inline VkShaderModuleCreateInfo shaderModuleCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
        };
    }

    inline VkRenderPassCreateInfo renderPassCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO
        };
    }

    inline VkFramebufferCreateInfo framebufferCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO
        };
    }

    inline VkCommandPoolCreateInfo commandPoolCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO
        };
    }

    inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool commandPool,
                                                                 VkCommandBufferLevel level,
                                                                 uint32_t bufferCount) {
        return {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = level,
            .commandBufferCount = bufferCount
        };
    }

    inline VkSemaphoreCreateInfo semaphoreCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
    }

    inline VkFenceCreateInfo fenceCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
        };
    }

    inline VkBufferCreateInfo bufferCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO
        };
    }
    inline VkBufferCreateInfo bufferCreateInfo(VkBufferUsageFlags usage,
                                               VkDeviceSize size) {
        return {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage
        };
    }

    inline VkMemoryAllocateInfo memoryAllocateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
        };
    }

    inline VkCommandBufferBeginInfo commandBufferBeginInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };
    }

    inline VkSubmitInfo submitInfo() {
        return {
            .sType =  VK_STRUCTURE_TYPE_SUBMIT_INFO
        };
    }

    inline VkPresentInfoKHR presentInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR
        };
    };

    inline VkRenderPassBeginInfo renderPassBeginInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO
        };
    }

    inline VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
        };
    }

} // End namespace vk::initializers


#endif //PROTOTYPE_ACTION_RPG_INITIALIZERS_HPP
