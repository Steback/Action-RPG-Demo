#ifndef PROTOTYPE_ACTION_RPG_MESHMODEL_HPP
#define PROTOTYPE_ACTION_RPG_MESHMODEL_HPP


#include <string>


namespace core {

    class MeshModel {
    public:
        explicit MeshModel(uint64_t modelID);

        uint64_t getModelID() const;

        void setModelID(uint64_t id);

    private:
        uint64_t m_modelID;
    };

} // namespace core


#endif //PROTOTYPE_ACTION_RPG_MESHMODEL_HPP
