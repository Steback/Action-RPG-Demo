#include "Instance.hpp"

namespace vk {

    Instance::Instance() = default;

    Instance::~Instance() = default;

    void Instance::init(VkInstanceCreateInfo &createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

        vk::resultValidation(vkCreateInstance(&createInfo, nullptr, &mInstance),
                             "Failed to create instance");
    }

    void Instance::destroy() {

    }

} // End namespace vk