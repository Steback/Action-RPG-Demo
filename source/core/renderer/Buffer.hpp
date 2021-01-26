#ifndef PROTOTYPE_ACTION_RPG_BUFFER_HPP
#define PROTOTYPE_ACTION_RPG_BUFFER_HPP


#include "vulkan/vulkan.h"


namespace vk {

    struct Buffer {
        VkBuffer buffer{};
        VkDeviceMemory deviceMemory{};
    };

    uint32_t findMemoryType(const VkPhysicalDevice& device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_BUFFER_HPP
