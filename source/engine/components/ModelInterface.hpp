#ifndef PROTOTYPE_ACTION_RPG_MODELINTERFACE_HPP
#define PROTOTYPE_ACTION_RPG_MODELINTERFACE_HPP


#include <string>
#include <memory>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"

#include "Transform.hpp"
#include "../resources/Model.hpp"
#include "../renderer/GraphicsPipeline.hpp"


namespace engine {

    struct MVP;

    class ModelInterface {
    public:
        explicit ModelInterface(uint64_t modelID, uint32_t entityID);

        engine::Model::Node& getNode(uint32_t id);

        std::vector<engine::Model::Node>& getNodes();

        std::string& getName();

        void render(vk::CommandBuffer& cmdBuffer, const std::shared_ptr<GraphicsPipeline>& pipeAnimation,
                    const std::shared_ptr<GraphicsPipeline>& pipeModel);

        void setModel(uint64_t modelID);

        uint32_t getRootNode();

        static void setLuaBindings(sol::table& table);

    private:
        std::shared_ptr<engine::Model> m_model;
        uint32_t m_entityID;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_MODELINTERFACE_HPP
