#ifndef PROTOTYPE_ACTION_RPG_MODEL_HPP
#define PROTOTYPE_ACTION_RPG_MODEL_HPP


#include <string>
#include <memory>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"

#include "../resources/ModelInterface.hpp"
#include "Transform.hpp"


namespace engine {

    struct MVP;

    class Model {
    public:
        explicit Model(uint64_t modelID, uint32_t entityID);

        engine::ModelInterface::Node& getNode(uint id);

        std::vector<engine::ModelInterface::Node>& getNodes();

        std::string& getName();

        void render(vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& layout);

        void setModel(uint64_t modelID);

        static void setLuaBindings(sol::table& table);

    private:
        std::shared_ptr<engine::ModelInterface> m_model;
        uint32_t m_entityID;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_MODEL_HPP
