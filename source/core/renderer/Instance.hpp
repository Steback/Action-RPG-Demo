#ifndef PROTOTYPE_ACTION_RPG_INSTANCE_HPP
#define PROTOTYPE_ACTION_RPG_INSTANCE_HPP


#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

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

        void pickPhysicalDevice(PhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

        void createSurface(GLFWwindow* window, VkSurfaceKHR& surface);

        void destroySurface(VkSurfaceKHR& surface);

    private:
        void setupDebugMessenger();

    private:
        VkInstance mInstance{};
        VkDebugUtilsMessengerEXT mDebugMessenger{};
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_INSTANCE_HPP
