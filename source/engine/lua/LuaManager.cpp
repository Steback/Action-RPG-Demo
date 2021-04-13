#include "LuaManager.hpp"

#include <utility>

#include "fmt/format.h"

#include "../Constants.hpp"


namespace engine {

    LuaManager::LuaManager() {
        m_state.open_libraries(sol::lib::base, sol::lib::math);
    }

    void LuaManager::scriptFile(const std::string &uri) {
        m_state.script_file(SCRIPTS_DIR + uri);
    }

    sol::state &LuaManager::getState() {
        return m_state;
    }

    void LuaManager::executeFunction(const std::string& name) {
#ifdef CORE_DEBUG
        sol::protected_function func = m_state[name];
        sol::protected_function_result result = func();

        if (!result.valid()) {
            sol::error error = result;
            fmt::print("{}\n", error.what());
        }
#else
        m_state[name]();
#endif

    }

} // namespace engine