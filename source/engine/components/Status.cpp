#include "Status.hpp"


namespace engine {

    Status::Status(uint32_t owner) : owner(owner), type(Type::ACTIVE) {  }

    Status::Type Status::getType() const {
        return type;
    }

    void Status::setType(Status::Type type_) {
        type = type_;
    }

} // namespace engine
