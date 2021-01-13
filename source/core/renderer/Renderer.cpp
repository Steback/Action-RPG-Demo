#include "Renderer.hpp"

#include "GLFW/glfw3.h"


namespace core {

    Renderer::Renderer() {
        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Prototype Action RPG",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Custom Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_2
        };

        std::vector<const char*> glfwExtensions = vk::getRequiredExtensions();

        VkInstanceCreateInfo instanceCreateInfo{
            .pApplicationInfo = &appInfo,
        };

        mInstance.init(instanceCreateInfo);
    }

    Renderer::~Renderer() = default;

    void Renderer::draw() {

    }

    void Renderer::clean() {

    }

} // End namespace core