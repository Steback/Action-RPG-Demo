#include <memory>

#include "GLFW/glfw3.h"

#include "core/logger/Logger.hpp"
#include "core/Application.hpp"


class App : public core::Application {
public:
    explicit App(const std::string& appName) : core::Application(appName) {

    }

    void init() override {
        auto enttID = m_registry.create();
        auto entity = m_scene->addEntity("Viking Room", enttID);

        m_registry.emplace<core::Transform>(enttID, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, 0.0f);
        m_registry.emplace<core::Model>(enttID, m_resourceManager->createModel("viking-room.obj"));
    }

    void update() override {
        m_scene->update(m_registry);
    }

    void draw() override {

    }

    void cleanup() override {
        auto view = m_registry.view<core::Model>();

        for (auto& entity : view) {
            auto& model = view.get<core::Model>(entity);
            model.clean();
        }
    }

private:
};


int main() {
    core::Logger logger;
    logger.init("App", "error.log");

    App app("Prototype Action RPG");

    try {
        app.run();
    } catch (const std::exception& ex) {
        logger.sendLog(core::ERROR, ex.what());
    }

    return EXIT_SUCCESS;
}