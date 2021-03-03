#ifndef PROTOTYPE_ACTION_RPG_MESHMODEL_HPP
#define PROTOTYPE_ACTION_RPG_MESHMODEL_HPP


#include <string>


namespace core {

    class MeshModel {
    public:
        MeshModel(std::string mModelName, uint mMeshNodeId);

        std::string& getModelName();

        uint& getMeshNodeID();

    private:
        std::string m_modelName;
        uint m_meshNodeID;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_MESHMODEL_HPP
