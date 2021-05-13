#include "ModelInterface.hpp"

#include "../Utilities.hpp"
#include "../Application.hpp"


namespace engine {

    ModelInterface::ModelInterface(uint64_t modelID, uint32_t entityID)
            : m_model(engine::Application::m_resourceManager->getModel(modelID)), m_entityID(entityID) {

    }

    engine::Model::Node &ModelInterface::getNode(uint32_t id) {
        return m_model->getNode(id);
    }

    std::vector<engine::Model::Node> &ModelInterface::getNodes() {
        return m_model->getNodes();
    }

    std::string &ModelInterface::getName() {
        return m_model->getName();
    }

    void ModelInterface::render(vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout) {
        MVP mvp = Application::m_renderer->m_mvp;


        for (auto& node : m_model->getNodes()) {
            if (node.mesh > 0) {
                auto& transform = Application::m_scene->getComponent<Transform>(m_entityID);
                glm::mat4 modelMatrix = transform.worldTransformMatrix() * node.matrix;
                auto& mesh = Application::m_resourceManager->getMesh(node.mesh);
                mvp.model = modelMatrix;

                glm::mat4 mvpMatrix = mvp.getMatrix();
                cmdBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(mvpMatrix), &mvpMatrix);

                vk::Buffer vertexBuffer[] = {mesh.getVertexBuffer()};
                vk::DeviceSize offsets[] = {0};
                cmdBuffer.bindVertexBuffers(0, 1, vertexBuffer, offsets);
                cmdBuffer.bindIndexBuffer(mesh.getIndexBuffer(), 0, vk::IndexType::eUint32);

                std::vector<vk::DescriptorSet> descriptorSetGroup = {
                        Application::m_renderer->getDescriptorSet(),
                        engine::Application::m_resourceManager->getTexture(mesh.getTextureId()).getDescriptorSet()
                };

                if (node.skin > -1)
                    descriptorSetGroup.push_back(m_model->getSkin(node.skin).descriptorSet);

                cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0,
                                             static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(),
                                             0,nullptr);

                cmdBuffer.drawIndexed(mesh.getIndexCount(), 1, 0, 0, 0);
            }
        }
    }

    void ModelInterface::setModel(uint64_t modelID) {
        m_model = engine::Application::m_resourceManager->getModel(modelID);
    }

    void ModelInterface::setLuaBindings(sol::table &table) {
        table.new_usertype<Model::Node>("Node",
                                        "id", &Model::Node::id,
                                        "name", &Model::Node::name,
                                        "position", &Model::Node::position,
                                        "rotation", &Model::Node::rotation,
                                        "scale", &Model::Node::scale,
                                        "children", &Model::Node::children,
                                        "mesh", &Model::Node::mesh,
                                        "parent", &Model::Node::parent);

        table.new_usertype<ModelInterface>("Model",
                                  sol::call_constructor, sol::constructors<Model(uint64_t, uint32_t)>(),
                                  "setModel", &ModelInterface::setModel,
                                  "getName", &ModelInterface::getName,
                                  "getNodes", &ModelInterface::getNodes,
                                  "getNode", &ModelInterface::getNode,
                                  "getRootNode", &ModelInterface::getRootNode);
    }

    uint32_t ModelInterface::getRootNode() {
        return m_model->getRootNode();
    }

} // namespace core
