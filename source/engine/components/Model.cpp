#include "Model.hpp"

#include "glm/gtx/matrix_decompose.hpp"

#include "../Utilities.hpp"
#include "../Application.hpp"


namespace engine {

    Model::Model(uint64_t modelID, uint32_t entityID)
            : m_model(engine::Application::m_resourceManager->getModel(modelID)), m_entityID(entityID) {

    }

    engine::ModelInterface::Node &Model::getNode(uint id) {
        return m_model->getNode(id);
    }

    engine::ModelInterface::Node &Model::getBaseMesh() {
        return m_model->getBaseMesh();
    }

    std::vector<engine::ModelInterface::Node> &Model::getNodes() {
        return m_model->getNodes();
    }

    std::string &Model::getName() {
        return m_model->getName();
    }

    void Model::render(vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout, const vk::DescriptorSet& set, MVP& mvp) {
        for (auto& node : m_model->getNodes()) {
            if (node.mesh > 0) {
                auto& transform = engine::Application::m_scene->getComponent<engine::Transform>(m_entityID);
                glm::mat4 modelMatrix;

                if (m_model->getBaseMesh().id == node.id) {
                    modelMatrix = transform.worldTransformMatrix();
                } else {
                    modelMatrix = transform.worldTransformMatrix() * node.matrix;
                }

                auto& mesh = Application::m_resourceManager->getMesh(node.mesh);
                mvp.model = modelMatrix;

                vk::Buffer vertexBuffer[] = {mesh.getVertexBuffer()};
                vk::DeviceSize offsets[] = {0};
                cmdBuffer.bindVertexBuffers(0, 1, vertexBuffer, offsets);
                cmdBuffer.bindIndexBuffer(mesh.getIndexBuffer(), 0, vk::IndexType::eUint32);

                glm::mat4 mvpMatrix = mvp.getMatrix();
                cmdBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(mvpMatrix), &mvpMatrix);

                std::array<vk::DescriptorSet, 2> descriptorSetGroup = {
                        set,
                        engine::Application::m_resourceManager->getTexture(mesh.getTextureId()).getDescriptorSet()
                };

                cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0,
                                             static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0,
                                             nullptr);

                cmdBuffer.drawIndexed(mesh.getIndexCount(), 1, 0, 0, 0);
            }
        }
    }

    void Model::setModel(uint64_t modelID) {
        m_model = engine::Application::m_resourceManager->getModel(modelID);
    }

    void Model::descomposeMatrix(Transform& transform) {
        glm::vec3 tempTranslate, tempScale, tempSkew;
        glm::vec4 tempPerspective;
        glm::quat tempOrientation;

        glm::decompose(m_model->getBaseMesh().matrix, tempScale, tempOrientation, tempTranslate, tempSkew, tempPerspective);

        transform.getPosition() += tempTranslate;
        transform.getRotation() += glm::eulerAngles(tempOrientation);
    }

    void Model::setLuaBindings(sol::table &table) {
        table.new_usertype<Model>("Model",
                                  sol::call_constructor, sol::constructors<Model(uint64_t, uint32_t)>(),
                                  "setModel", &Model::setModel,
                                  "getName", &Model::getName,
                                  "descomposeMatrix", &Model::descomposeMatrix);
    }

} // namespace core
