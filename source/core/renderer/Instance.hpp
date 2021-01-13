#ifndef PROTOTYPE_ACTION_RPG_INSTANCE_HPP
#define PROTOTYPE_ACTION_RPG_INSTANCE_HPP


#include "vulkan/vulkan.h"
#include "Utils.hpp"


namespace vk {

    class Instance {
    public:
        Instance();
        ~Instance();
        void init(VkInstanceCreateInfo& createInfo);
        void destroy();

    private:
        VkInstance mInstance{};
    };

} // End namespace vk


#endif //PROTOTYPE_ACTION_RPG_INSTANCE_HPP
