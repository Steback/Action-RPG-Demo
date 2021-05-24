#include "ModelInterface.hpp"

#include "../Utilities.hpp"
#include "../Application.hpp"


const std::vector<std::string> conflictsNodes = {
        "Bow",
        "fould_A",
        "fould_B",
        "helmet_A",
        "helmet_B",
        "helmet_C",
        "hood_A",
        "skeleton_mesh"
};

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

    void ModelInterface::render(vk::CommandBuffer& cmdBuffer, const std::shared_ptr<GraphicsPipeline>& pipeAnimation,
                                const std::shared_ptr<GraphicsPipeline>& pipeModel) {
        for (auto& node : m_model->getNodes()) {
            if (node.mesh > 0) {
                bool haveSkin = node.skin > -1;
                std::shared_ptr<GraphicsPipeline> pipeline = ( haveSkin ? pipeAnimation : pipeModel);
                pipeline->bind(cmdBuffer);

                auto& transform = Application::m_scene->getComponent<Transform>(m_entityID);
                // TODO: All the skeletons models have a error when applying transforms with some nodes.
                // So my "best" solution to this problem. I create a vector with all the name of the conflict nodes and
                // not apply the transform to them. If anyone can tell what's the error or how to solve it, I really appreciate that.
                bool conflictNode = std::find(conflictsNodes.begin(), conflictsNodes.end(), node.name) == conflictsNodes.end();

                Application::m_renderer->m_mvp.model = transform.worldTransformMatrix() * (conflictNode ? node.getMatrix(m_model) : node.matrix);
                cmdBuffer.pushConstants(pipeline->getLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(MVP), &Application::m_renderer->m_mvp);

                if (haveSkin && !Application::m_editor)
                    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->getLayout(), 2, 1, &m_model->getSkin(node.skin).descriptorSet,
                                                 0, nullptr);

                auto& mesh = Application::m_resourceManager->getMesh(node.mesh);

                vk::Buffer vertexBuffer[] = {mesh.getVertexBuffer()};
                vk::DeviceSize offsets[] = {0};
                cmdBuffer.bindVertexBuffers(0, 1, vertexBuffer, offsets);
                cmdBuffer.bindIndexBuffer(mesh.getIndexBuffer(), 0, vk::IndexType::eUint32);

                vk::DescriptorSet texture = Application::m_resourceManager->getTexture(mesh.getTextureId()).getDescriptorSet();
                cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->getLayout(), 1, 1,
                                             &texture, 0, nullptr);

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
