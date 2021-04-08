#ifndef PROTOTYPE_ACTION_RPG_INSTANCE_HPP
#define PROTOTYPE_ACTION_RPG_INSTANCE_HPP

#include <vector>
#include <map>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

#include "../Constants.hpp"


namespace core {

    class Instance {
    public:
        Instance();

        explicit Instance(const vk::ApplicationInfo& appInfo);

        ~Instance();

        void destroy();

        [[nodiscard]] vk::Instance getInstance() const;

        vk::PhysicalDevice selectPhysicalDevice(const std::vector<const char *>& enabledExtensions);

        vk::SurfaceKHR createSurface(GLFWwindow* window);

        void destroy(vk::SurfaceKHR surface);

    private:
        vk::Instance m_instance{};
#ifdef CORE_DEBUG
        vk::DebugUtilsMessengerEXT m_debugMessenger{};

    public:
        std::unordered_map<uint32_t, std::string> m_debugMessages;
#endif
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_INSTANCE_HPP
