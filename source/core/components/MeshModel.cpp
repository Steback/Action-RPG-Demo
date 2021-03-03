#include "MeshModel.hpp"


namespace core {

    MeshModel::MeshModel(std::string modelName, uint meshNodeId) :
            m_modelName(std::move(modelName)), m_meshNodeID(meshNodeId) {}

    std::string &MeshModel::getModelName() {
        return m_modelName;
    }

    uint &MeshModel::getMeshNodeID() {
        return m_meshNodeID;
    }

} // namespace core
