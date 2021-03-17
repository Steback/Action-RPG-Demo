#ifndef PROTOTYPE_ACTION_RPG_APPLICATION_HPP
#define PROTOTYPE_ACTION_RPG_APPLICATION_HPP


#include <memory>

#include "GLFW/glfw3.h"
#include "entt/entt.hpp"

#include "window/Window.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "scene/Scene.hpp"
#include "components/Transform.hpp"
#include "resources/ResourceManager.hpp"


namespace core {

    class Application {
    public:
        explicit Application(const std::string& appName, bool drawGrid = false);

        ~Application();

        void run();

        void loop();

        void render();

        void shutdown();

        virtual void init() = 0;

        virtual void update() = 0;

        virtual void draw() = 0;

        virtual void cleanup() = 0;

    public:
        static std::unique_ptr<core::Renderer> renderer;
        static core::ResourceManager* resourceManager;

    protected:
        std::unique_ptr<core::Window> m_window;
        std::unique_ptr<core::Scene> m_scene;
        vk::Device* m_device{};
        vk::Instance m_instance;
        VkSurfaceKHR m_surface{};
        float m_lastTime{}, m_deltaTime{};
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_APPLICATION_HPP
