#include "Renderer.hpp"

#include "GLFW/glfw3.h"

#include "Utils.hpp"


namespace core {

    Renderer::Renderer() {
        vk::ApplicationInfo applicationInfo{
                .pApplicationName = "Prototype Action RPG",
                .applicationVersion = 1,
                .pEngineName = "Custom Engine",
                .engineVersion = 1,
                .apiVersion = VK_API_VERSION_1_2 };

        std::vector<const char*> glfwExtensions = core::getRequiredExtensions();

        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;

        if (enableValidationLayers) {
            mValidationLayers = Debug({"VK_LAYER_KHRONOS_validation" } );

            if (!mValidationLayers.checkValidationLayerSupport()) {
                spdlog::throw_spdlog_ex("[Renderer] Validation layers requested, but not available!");
            }

            mValidationLayers.populateDebugMessengerCreateInfo(debugCreateInfo);
        }

        vk::InstanceCreateInfo instanceCreateInfo{
//                .pNext = enableValidationLayers ? &debugCreateInfo : nullptr,
                .pApplicationInfo = &applicationInfo,
                .enabledLayerCount = enableValidationLayers ? mValidationLayers.getValidationLayersCount() : 0,
                .ppEnabledLayerNames = enableValidationLayers ? mValidationLayers.getValidationLayers() : nullptr,
                .enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size()),
                .ppEnabledExtensionNames = glfwExtensions.data() };

        core::resultValidation(vk::createInstance(&instanceCreateInfo, nullptr, &mInstance),
                               "Failed to create instance");

        if (enableValidationLayers) {
            mValidationLayers.init(mInstance);
        }
    }

    Renderer::~Renderer() = default;

    void Renderer::draw() {

    }

    void Renderer::clean() {
        mValidationLayers.clean(mInstance);
        mInstance.destroy(nullptr);
    }

} // End namespace core