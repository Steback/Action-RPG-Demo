#ifndef PROTOTYPE_ACTION_RPG_APPLICATION_HPP
#define PROTOTYPE_ACTION_RPG_APPLICATION_HPP


#include <memory>

#include "GLFW/glfw3.h"
#include "entt/entt.hpp"

#include "window/Window.hpp"
#include "renderer/RenderDevice.hpp"
#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "scene/Scene.hpp"
#include "components/Transform.hpp"
#include "resources/ResourceManager.hpp"


namespace core {

    class Application {
    public:
        explicit Application(const std::string& appName, const glm::vec4& clearColor = {glm::vec3(0.0f), 1.0f}, bool drawGrid = false);

        ~Application();

        void run();

        void loop();

        void shutdown();

        virtual void init() = 0;

        virtual void update() = 0;

        virtual void drawUI() = 0;

        virtual void cleanup() = 0;

    public:
        static std::unique_ptr<core::RenderDevice> m_renderer;
        static std::unique_ptr<core::ResourceManager> m_resourceManager;
        static std::unique_ptr<core::Scene> m_scene;

    protected:
        std::shared_ptr<core::Window> m_window;
        std::shared_ptr<vkc::Device> m_device;
        std::shared_ptr<vkc::Instance> m_instance;
        float m_lastTime{}, m_deltaTime{};
        glm::vec4 m_clearColor;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_APPLICATION_HPP
