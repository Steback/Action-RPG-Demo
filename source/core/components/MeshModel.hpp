#ifndef PROTOTYPE_ACTION_RPG_MESHMODEL_HPP
#define PROTOTYPE_ACTION_RPG_MESHMODEL_HPP


#include <string>


namespace core {

    class MeshModel {
    public:
        explicit MeshModel(uint modelID);

        uint getModelID();

        void setModelID(uint id);

    private:
        uint m_modelID;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_MESHMODEL_HPP
