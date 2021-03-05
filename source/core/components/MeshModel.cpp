#include "MeshModel.hpp"


namespace core {

    MeshModel::MeshModel(std::string modelName) : m_modelName(std::move(modelName)) {}

    std::string &MeshModel::getModelName() {
        return m_modelName;
    }

} // namespace core
