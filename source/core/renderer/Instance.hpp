#ifndef PROTOTYPE_ACTION_RPG_INSTANCE_HPP
#define PROTOTYPE_ACTION_RPG_INSTANCE_HPP

#include <vector>

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "../Constants.hpp"


namespace vkc {

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
#ifdef CORE_DEBUG
        void setupDebugMessenger();
#endif

    private:
        VkInstance m_instance{};
#ifdef CORE_DEBUG
        VkDebugUtilsMessengerEXT debugMessenger{};
#endif
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_INSTANCE_HPP
