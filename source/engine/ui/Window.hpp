#ifndef PROTOTYPE_ACTION_RPG_UI_WINDOW_HPP
#define PROTOTYPE_ACTION_RPG_UI_WINDOW_HPP

#include <string>
#include <functional>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "glm/glm.hpp"


namespace engine::ui {

    enum WindowFlags {
        close = 1 << 0,
        fixPosition = 1 << 1,
    };

    class Window {
    public:
        Window(std::string name, float with, float height, uint32_t flags);

        std::string getName();

        void setName(const std::string& name);

        [[nodiscard]] float getWidth() const;

        void setWidth(float width);

        [[nodiscard]] float getHeight() const;

        void setHeight(float height);

        void drawLua(const sol::function& f, int flags = 0);

        void draw(std::function<void()> f, int flags = 0);

        bool isOpen() const;

        void setState(bool state);

        void setPosition(float x, float y);

        static void setLuaClass(sol::table& state);

    private:
        std::string m_name;
        float m_width{}, m_height{};
        glm::vec2 m_position{};
        bool m_open{false};
        uint32_t m_flags{};
    };

} // namespace engine::ui


#endif //PROTOTYPE_ACTION_RPG_UI_WINDOW_HPP
