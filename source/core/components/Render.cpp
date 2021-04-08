#include "Render.hpp"

#include "../Utilities.hpp"
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

    void Render::render(vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout, const vk::DescriptorSet& set, MVP& mvp) {
        for (auto& node : m_model->getNodes()) {
            if (node.mesh > 0) {
                auto& transform = core::Application::m_scene->getComponent<core::Transform>(m_entityID);
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

                cmdBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(MVP), &mvp);

                std::array<vk::DescriptorSet, 2> descriptorSetGroup = {
                        set,
                        core::Application::m_resourceManager->getTexture(mesh.getTextureId()).getDescriptorSet()
                };

                cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0,
                                             static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0,
                                             nullptr);

                cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, 1, &set, 0,
                                             nullptr);

                cmdBuffer.drawIndexed(mesh.getIndexCount(), 1, 0, 0, 0);
            }
        }
    }

    void Render::setModel(uint64_t modelID) {
        m_model = core::Application::m_resourceManager->getModel(modelID);
    }

} // namespace core
