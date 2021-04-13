#ifndef PROTOTYPE_ACTION_RPG_UI_WINDOW_HPP
#define PROTOTYPE_ACTION_RPG_UI_WINDOW_HPP

#include <string>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"


#define WINDOW_ARGS std::string name, float with, float height, uint32_t flags


namespace engine::ui {

    enum WindowFlags {
        reqOpen = 1 << 0
    };

    class Window {
    public:
        Window(WINDOW_ARGS);

        std::string getName();

        void setName(const std::string& name);

        [[nodiscard]] float getWidth() const;

        void setWidth(float width);

        [[nodiscard]] float getHeight() const;

        void setHeight(float height);

        void draw(const sol::function& f);

        bool isOpen() const;

        static void setLuaClass(sol::table& state);

    private:
        std::string m_name;
        float m_width{}, m_height{};
        bool m_open{true};
        uint32_t m_flags{};
    };

} // namespace engine::ui


#endif //PROTOTYPE_ACTION_RPG_UI_WINDOW_HPP
