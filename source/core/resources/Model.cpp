#include "Model.hpp"

#include <utility>

#include "glm/glm.hpp"

#include "../Utilities.hpp"


namespace core {

    Model::Model() = default;

    Model::Model(std::vector<Node> nodes)
            : m_nodes(std::move(nodes)) {

    }

    Model::~Model() = default;

    Model::Node &Model::getNode(uint id) {
        return m_nodes[id];
    }

    std::vector<Model::Node>& Model::getNodes() {
        return m_nodes;
    }

    void Model::cleanup() {

    }

} // namespace core
