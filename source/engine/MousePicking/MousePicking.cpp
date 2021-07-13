#include "MousePicking.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

#include "../window/Window.hpp"
#include "../physcis/PhysicsEngine.hpp"
#include "../Application.hpp"


namespace engine {

    MousePicking::MousePicking() = default;

    MousePicking::MousePicking(std::shared_ptr<Window> window) : window(std::move(window)) {

    }

    void MousePicking::mouseRayCast() {
        Camera& camera = Application::m_scene->getCamera();
        glm::vec2 mousePos = window->getCurrentMousePos();

        glm::vec4 rayStart = {(mousePos.x / (float)window->getSize().width - 0.5f) * 2.0f,
                              (mousePos.y / (float)window->getSize().height - 0.5f) * 2.0f,
                              -1.0f,
                              1.0f};
        glm::vec4 rayEnd = {(mousePos.x / (float)window->getSize().width - 0.5f) * 2.0f,
                              (mousePos.y / (float)window->getSize().height - 0.5f) * 2.0f,
                              0.0f,
                              1.0f};

        glm::mat4 invProj = glm::inverse(camera.getProjection(window->aspect()));
        glm::mat4 inView = glm::inverse(camera.getView());

        glm::vec4 rayStartCamera = invProj * rayStart; rayStartCamera /= rayStartCamera.w;
        glm::vec4 rayStartWorld = inView * rayStartCamera; rayStartWorld /= rayStartWorld.w;
        glm::vec4 rayEndCamera = invProj * rayEnd; rayEndCamera /= rayEndCamera.w;
        glm::vec4 ratEndWorld = inView * rayEndCamera; ratEndWorld /= ratEndWorld.w;

        glm::vec3 rayDirWorld(ratEndWorld - rayStartWorld);

        origin = glm::vec3(rayStartWorld);
        direction = glm::normalize(rayDirWorld);
    }

    void MousePicking::pick() {
        mouseRayCast();

        glm::vec3 end = origin + direction * 100.0f;

        btCollisionWorld::ClosestRayResultCallback rayResultCallback(
                {origin.x, origin.y, origin.z},
                {end.x, end.y, end.z}
        );

        Application::physicsEngine->getDynamicsWorld()->rayTest(
                {origin.x, origin.y, origin.z},
                {end.x, end.y, end.z},
                rayResultCallback
        );

        if (rayResultCallback.hasHit()) {
            entityPicked = Application::m_scene->getEntity(rayResultCallback.m_collisionObject->getUserIndex());
        }
    }

    Entity &MousePicking::getEntityPicked() {
        return entityPicked;
    }

    const glm::vec3 &MousePicking::getOrigin() const {
        return origin;
    }

    const glm::vec3 &MousePicking::getDirection() const {
        return direction;
    }

    glm::vec3 MousePicking::getDirectionAugmented() const {
        return origin + direction * Application::m_scene->getCamera().getDistance();
    }

    void MousePicking::setLuaBindings(sol::state &state) {
        sol::table mouse = state["mouse"].get_or_create<sol::table>();
        mouse.set_function("getEntityPicked", &MousePicking::getEntityPicked, this);
        mouse.set_function("getOrigin", &MousePicking::getOrigin, this);
        mouse.set_function("getDirection", &MousePicking::getDirection, this);
        mouse.set_function("getDirectionAugmented", &MousePicking::getDirectionAugmented, this);
    }

    bool MousePicking::leftClickPressed() const {
        return window->mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
    }
}