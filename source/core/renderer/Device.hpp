#ifndef PROTOTYPE_ACTION_RPG_DEVICE_HPP
#define PROTOTYPE_ACTION_RPG_DEVICE_HPP


#include "vulkan/vulkan.h"
#include "PhysicalDevice.hpp"


namespace vk {

    class Device {
    public:
        Device();

        ~Device();

        void init(const PhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

        void destroy();

    private:
        VkDevice mDevice{};
        VkQueue mPresentQueue{};
        VkQueue mGraphicsQueue{};
        PhysicalDevice mPhysicalDevice;
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_DEVICE_HPP
