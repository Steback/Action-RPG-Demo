#ifndef PROTOTYPE_ACTION_RPG_RENDERER_HPP
#define PROTOTYPE_ACTION_RPG_RENDERER_HPP


#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_NO_NODISCARD_WARNINGS
#include "vulkan/vulkan.hpp"

#include "Debug.hpp"


namespace core {

    class Renderer {
    public:
        Renderer();

        ~Renderer();

        void draw();

        void clean();

    private:
        vk::Instance mInstance{};
        Debug mValidationLayers;
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDERER_HPP
