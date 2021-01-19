#ifndef PROTOTYPE_ACTION_RPG_DEVICE_HPP
#define PROTOTYPE_ACTION_RPG_DEVICE_HPP


#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "PhysicalDevice.hpp"
#include "SwapChain.hpp"


namespace vk {

    class Device {
    public:
        Device();

        ~Device();

        void init(const PhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

        void destroy();

        void createSwapChain(SwapChain& swapChain, GLFWwindow* window, VkSurfaceKHR surface);

        void destroySwapChain(SwapChain& swapChain);

        void createImageViews(SwapChain& swapChain);

        void destroyImageViews(SwapChain& swapChain);

        void createGraphicsPipeline(VkPipelineLayout& pipelineLayout, const VkExtent2D& swapChainExtend);

        void destroyGraphicsPipeline(VkPipelineLayout& pipelineLayout);

        VkShaderModule createShaderModule(const std::vector<char>& code);

        void destroyShaderModule(VkShaderModule& shader);

    private:
        VkDevice mDevice{};
        VkQueue mPresentQueue{};
        VkQueue mGraphicsQueue{};
        PhysicalDevice mPhysicalDevice;
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_DEVICE_HPP
