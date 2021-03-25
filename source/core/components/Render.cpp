#include "Render.hpp"

#include <utility>

#include "../Application.hpp"


namespace core {

    Render::Render(uint64_t modelID, uint32_t entityID)
            : m_model(core::Application::m_resourceManager->getModel(modelID)), m_entityID(entityID) {

    }

    core::Model::Node &Render::getNode(uint id) {
        return m_model->getNode(id);
    }

    core::Model::Node &Render::getBaseMesh() {
        return m_model->getBaseMesh();
    }

    std::vector<core::Model::Node> &Render::getNodes() {
        return m_model->getNodes();
    }

    std::string &Render::getName() {
        return m_model->getName();
    }

    void Render::render() {
        for (auto& node : m_model->getNodes()) {
            if (node.mesh > 0) {
                auto& transform = core::Application::m_scene->getComponent<core::Transform>(m_entityID);
                glm::mat4 modelMatrix;

                if (m_model->getBaseMesh().id == node.id) {
                    modelMatrix = transform.worldTransformMatrix();
                } else {
                    modelMatrix = transform.worldTransformMatrix() * node.matrix;
                }

                core::Application::m_renderer->renderMesh(core::Application::m_resourceManager->getMesh(node.mesh), modelMatrix);
            }
        }
    }

    void Render::setModel(uint64_t modelID) {
        m_model = core::Application::m_resourceManager->getModel(modelID);
    }

} // namespace core
