#ifndef PROTOTYPE_ACTION_RPG_DEVICE_HPP
#define PROTOTYPE_ACTION_RPG_DEVICE_HPP


#include "vulkan/vulkan.h"
#include "PhysicalDevice.hpp"


namespace vk {

    class Device {
    public:
        Device();

        ~Device();

        void init(const PhysicalDevice& physicalDevice);

        void destroy();

    private:
        VkDevice mDevice{};
        VkQueue graphicsQueue{};
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_DEVICE_HPP
