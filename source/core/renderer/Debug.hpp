#ifndef PROTOTYPE_ACTION_RPG_DEBUG_HPP
#define PROTOTYPE_ACTION_RPG_DEBUG_HPP


#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_NO_NODISCARD_WARNINGS
#include "vulkan/vulkan.hpp"


namespace core {

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    class Debug {
    public:
        Debug();

        explicit Debug(std::vector<const char*> layerNames);

        ~Debug();

        void init(vk::Instance& instance);

        bool checkValidationLayerSupport();

        uint32_t getValidationLayersCount();

        const char** getValidationLayers();

        void clean(vk::Instance& instance);

        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        static void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);

    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
                void *userData );

        static VkResult createDebugUtilsMessengerEXT(VkInstance instance,
                                              VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                              VkAllocationCallbacks* pAllocator,
                                              VkDebugUtilsMessengerEXT* pDebugMessenger);

        static void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                           VkDebugUtilsMessengerEXT debugMessenger,
                                           const VkAllocationCallbacks* pAllocator);

    private:
        std::vector<const char*> mValidationLayers;
        std::vector<const char*> mExtensions;
        VkDebugUtilsMessengerEXT mDebugMessenger{};
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_DEBUG_HPP
