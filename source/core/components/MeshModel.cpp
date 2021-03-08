#include "MeshModel.hpp"


namespace core {

    MeshModel::MeshModel(uint64_t modelID) : m_modelID(modelID) {}

    uint64_t MeshModel::getModelID() const {
        return m_modelID;
    }

    void MeshModel::setModelID(uint64_t id) {
        m_modelID = id;
    }

} // namespace core
