#ifndef PROTOTYPE_ACTION_RPG_INSTANCE_HPP
#define PROTOTYPE_ACTION_RPG_INSTANCE_HPP


#include "vulkan/vulkan.h"
#include "Utils.hpp"
#include "Debug.hpp"
#include "PhysicalDevice.hpp"


namespace vk {

    class Instance {
    public:
        Instance();

        ~Instance();

        void init(VkApplicationInfo& appInfo);

        void destroy();

        VkInstance& operator*();

        void pickPhysicalDevice(PhysicalDevice& physicalDevice);

    private:
        void setupDebugMessenger();

    private:
        VkInstance mInstance{};
        VkDebugUtilsMessengerEXT mDebugMessenger{};
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_INSTANCE_HPP
