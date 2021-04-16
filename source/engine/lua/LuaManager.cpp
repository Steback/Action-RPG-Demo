#include "LuaManager.hpp"

#include <filesystem>

#include "fmt/format.h"

#include "../Constants.hpp"


namespace engine {

    LuaManager::LuaManager() {
        m_state.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::package, sol::lib::io,
            sol::lib::table, sol::lib::os, sol::lib::bit32);
    }

    void LuaManager::scriptFile(const std::string &uri) {
        m_state.script_file(SCRIPTS_DIR + m_dirName + '/' + uri);
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

    void LuaManager::executeFunction(const sol::function& f) {
#ifdef CORE_DEBUG
        const sol::protected_function& func = f;
        sol::protected_function_result result = func();

        if (!result.valid()) {
            sol::error error = result;
            fmt::print("{}\n", error.what());
        }
#else
        f();
#endif
    }

    void LuaManager::setScriptsDir(const std::string& name) {
        m_dirName = name;

        const std::string package_path = m_state["package"]["path"];
        m_state["package"]["path"] = package_path + ";../scripts/" + m_dirName + "/?.lua";
    }

} // namespace engine