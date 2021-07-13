#ifndef ACTION_RPG_DEMO_MOUSEPICKING_HPP
#define ACTION_RPG_DEMO_MOUSEPICKING_HPP


#include <memory>

#include "glm/glm.hpp"

#include "../scene/Scene.hpp"


namespace sol {
    class state;
}

namespace engine {

    class Window;

    class MousePicking {
    public:
        MousePicking();

        explicit MousePicking(std::shared_ptr<Window> window);

        void mouseRayCast();

        void pick();

        Entity& getEntityPicked();

        [[nodiscard]] const glm::vec3 &getOrigin() const;

        [[nodiscard]] const glm::vec3 &getDirection() const;

        [[nodiscard]] glm::vec3 getDirectionAugmented() const;

        [[nodiscard]] bool leftClickPressed() const;

        void setLuaBindings(sol::state& state);

    private:
        std::shared_ptr<Window> window{};
        Entity entityPicked{};
        glm::vec3 origin{};
        glm::vec3 direction{};
    };

} // namespace engine


#endif //ACTION_RPG_DEMO_MOUSEPICKING_HPP
