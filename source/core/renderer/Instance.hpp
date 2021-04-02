#ifndef PROTOTYPE_ACTION_RPG_INSTANCE_HPP
#define PROTOTYPE_ACTION_RPG_INSTANCE_HPP

#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

#include "../Constants.hpp"


namespace vkc {

    class Instance {
    public:
        Instance();

        ~Instance();

        void init(const vk::ApplicationInfo& appInfo);

        void cleanup();

        [[nodiscard]] vk::Instance getInstance() const;

        vk::PhysicalDevice selectPhysicalDevice(const std::vector<const char *>& enabledExtensions);

        vk::SurfaceKHR createSurface(GLFWwindow* window);

        void destroy(vk::SurfaceKHR surface);

    private:
        vk::Instance m_instance{};
#ifdef CORE_DEBUG
        vk::DebugUtilsMessengerEXT m_debugMessenger{};
#endif
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_INSTANCE_HPP
