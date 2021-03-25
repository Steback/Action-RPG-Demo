#ifndef PROTOTYPE_ACTION_RPG_RENDER_HPP
#define PROTOTYPE_ACTION_RPG_RENDER_HPP


#include <string>
#include <memory>


#include "../resources/Model.hpp"


namespace core {

    class Render {
    public:
        explicit Render(uint64_t modelID, uint32_t entityID);

        core::Model::Node& getNode(uint id);

        core::Model::Node& getBaseMesh();

        std::vector<core::Model::Node>& getNodes();

        std::string& getName();

        void render();

        void setModel(uint64_t modelID);

    private:
        std::shared_ptr<core::Model> m_model;
        uint32_t m_entityID;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_RENDER_HPP
