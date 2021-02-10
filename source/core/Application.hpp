#ifndef PROTOTYPE_ACTION_RPG_APPLICATION_HPP
#define PROTOTYPE_ACTION_RPG_APPLICATION_HPP


#include <memory>

#include "GLFW/glfw3.h"
#include "entt/entt.hpp"

#include "logger/Logger.hpp"
#include "window/Window.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "scene/Scene.hpp"
#include "components/Transform.hpp"
#include "texture/ResourceManager.hpp"


namespace core {

    class Application {
    public:
        explicit Application(const std::string& appName);

        ~Application();

        void run();

        void loop();

        void destroy();

        virtual void init() = 0;

        virtual void update() = 0;

        virtual void draw() = 0;

        virtual void cleanup() = 0;

    protected:
        std::unique_ptr<core::Window> m_window;
        std::unique_ptr<core::Renderer> m_renderer;
        core::ResourceManager* m_resourceManager;
        std::unique_ptr<core::Scene> m_scene;
        vk::Device* m_device{};
        vk::Instance m_instance;
        VkSurfaceKHR m_surface{};
        entt::registry m_registry;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_APPLICATION_HPP
