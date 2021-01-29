#ifndef PROTOTYPE_ACTION_RPG_INSTANCE_HPP
#define PROTOTYPE_ACTION_RPG_INSTANCE_HPP


#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "Debug.hpp"


namespace vk {

    class Instance {
    public:
        Instance();

        ~Instance();

        void init(VkApplicationInfo& appInfo);

        void destroy();

        VkInstance& operator*();

        void pickPhysicalDevice(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface,
                                const std::vector<const char *>& enabledExtensions);

        void createSurface(GLFWwindow* window, VkSurfaceKHR& surface);

        void destroySurface(VkSurfaceKHR& surface);

    private:
        void setupDebugMessenger();

    private:
        VkInstance m_instance{};
        VkDebugUtilsMessengerEXT debugMessenger{};
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_INSTANCE_HPP
