#ifndef PROTOTYPE_ACTION_RPG_MESHMODEL_HPP
#define PROTOTYPE_ACTION_RPG_MESHMODEL_HPP


#include <string>


namespace core {

    class MeshModel {
    public:
        MeshModel(std::string mModelName);

        std::string& getModelName();

    private:
        std::string m_modelName;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_MESHMODEL_HPP
