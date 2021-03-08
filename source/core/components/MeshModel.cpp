#include "MeshModel.hpp"


namespace core {

    MeshModel::MeshModel(uint modelID) : m_modelID(modelID) {}

    uint MeshModel::getModelID() {
        return m_modelID;
    }

    void MeshModel::setModelID(uint id) {
        m_modelID = id;
    }

} // namespace core
