#ifndef PROTOTYPE_ACTION_RPG_MODEL_HPP
#define PROTOTYPE_ACTION_RPG_MODEL_HPP


#include <string>
#include <memory>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"

#include "../resources/ModelInterface.hpp"


namespace core {

    struct MVP;

    class Model {
    public:
        explicit Model(uint64_t modelID, uint32_t entityID);

        core::ModelInterface::Node& getNode(uint id);

        core::ModelInterface::Node& getBaseMesh();

        std::vector<core::ModelInterface::Node>& getNodes();

        std::string& getName();

        void render(vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout, const vk::DescriptorSet& set, MVP& mvp);

        void setModel(uint64_t modelID);

    private:
        std::shared_ptr<core::ModelInterface> m_model;
        uint32_t m_entityID;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_MODEL_HPP
