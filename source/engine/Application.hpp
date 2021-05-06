#ifndef PROTOTYPE_ACTION_RPG_APPLICATION_HPP
#define PROTOTYPE_ACTION_RPG_APPLICATION_HPP


#include <memory>

#include "GLFW/glfw3.h"
#include "entt/entt.hpp"

#include "window/Window.hpp"
#include "renderer/RenderEngine.hpp"
#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "scene/Scene.hpp"
#include "components/Transform.hpp"
#include "resources/ResourceManager.hpp"
#include "lua/LuaManager.hpp"
#include "ui/UIRender.hpp"
#include "threads/ThreadPool.hpp"


namespace engine {

    class CommandList;
    class GraphicsPipeline;

    class Application {
    public:
        explicit Application(const std::string& appName, const glm::vec4& clearColor = {glm::vec3(0.0f), 1.0f});

        ~Application();

        void run();

        void loop();

        void shutdown();

        virtual void init() = 0;

        virtual void update() = 0;

        virtual void drawUI() = 0;

        virtual void cleanup() = 0;

        virtual void renderCommands(vk::CommandBuffer& cmdBuffer) = 0;

        [[nodiscard]] float getDeltaTime() const;

    public:
        static std::unique_ptr<engine::RenderEngine> m_renderer;
        static std::unique_ptr<engine::ResourceManager> m_resourceManager;
        static std::unique_ptr<engine::Scene> m_scene;
        static std::unique_ptr<ThreadPool> m_threadPool;

    protected:
        std::shared_ptr<engine::Window> m_window;
        std::shared_ptr<engine::Device> m_device;
        std::shared_ptr<engine::Instance> m_instance;
        float m_lastTime{}, m_deltaTime{};
        glm::vec4 m_clearColor;
        std::shared_ptr<GraphicsPipeline> m_pipeline;
        std::shared_ptr<CommandList> m_commands;
        UIRender m_ui;
        LuaManager m_luaManager;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_APPLICATION_HPP
