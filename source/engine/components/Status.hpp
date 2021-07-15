#ifndef ACTION_RPG_DEMO_SOURCE_ENGINE_COMPONENTS_STATUS_HPP
#define ACTION_RPG_DEMO_SOURCE_ENGINE_COMPONENTS_STATUS_HPP


#include <cstdint>


namespace engine {

    class Status {
    public:
        enum Type {
            ACTIVE = 0,
            NO_ACTIVE = 1
        };

    public:
        explicit Status(uint32_t owner);

        [[nodiscard]] Type getType() const;

        void setType(Type type);

    private:
        Type type;
        uint32_t owner;
    };

} // namespace engine


#endif //ACTION_RPG_DEMO_SOURCE_ENGINE_COMPONENTS_STATUS_HPP
