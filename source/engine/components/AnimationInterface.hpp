#ifndef PROTOTYPE_ACTION_RPG_ANIMATIONINTERFACE_HPP
#define PROTOTYPE_ACTION_RPG_ANIMATIONINTERFACE_HPP


#include <memory>
#include <vector>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

#include "../resources/Animation.hpp"


namespace engine {

    class Animation;
    class Model;

    class AnimationInterface {
    public:
        AnimationInterface();

        explicit AnimationInterface(std::shared_ptr<Model> model, std::vector<uint32_t> animationList);

        void update(float deltaTime);

        static void setLuaBindings(sol::table& table);

    public:
        std::vector<uint32_t> animationsList;
        Animation::Type currentAnimation{Animation::Type::idle};
        std::shared_ptr<Animation> animation;
        std::shared_ptr<Model> model;
        bool reset{};
        bool loop{true};
    };

} // namespace engine


#endif //PROTOTYPE_ACTION_RPG_ANIMATIONINTERFACE_HPP
