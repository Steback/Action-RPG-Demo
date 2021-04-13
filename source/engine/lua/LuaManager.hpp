#ifndef PROTOTYPE_ACTION_RPG_LUAMANAGER_HPP
#define PROTOTYPE_ACTION_RPG_LUAMANAGER_HPP


#include <string>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace engine {

    class LuaManager {
    public:
        LuaManager();

        void scriptFile(const std::string& uri);

        sol::state& getState();

        void executeFunction(const std::string& name);

        template<typename T>
        T get(const std::string& name) {
            return static_cast<T>(m_state[name]);
        }

    private:
        sol::state m_state;
    };

} // namespace engine


#endif //PROTOTYPE_ACTION_RPG_LUAMANAGER_HPP
