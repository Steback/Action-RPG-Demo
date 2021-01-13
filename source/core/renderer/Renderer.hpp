#ifndef PROTOTYPE_ACTION_RPG_RENDERER_HPP
#define PROTOTYPE_ACTION_RPG_RENDERER_HPP


#include "Instance.hpp"


namespace core {

    class Renderer {
    public:
        Renderer();

        ~Renderer();

        void draw();

        void clean();

    private:
        vk::Instance mInstance;
    };

} // End namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDERER_HPP
