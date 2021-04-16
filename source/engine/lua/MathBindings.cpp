#include "MathBindings.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"


namespace engine::lua {

    void setMathBindings(sol::state& state) {
        sol::table eMath = state.create_table("glm");

        eMath.new_usertype<glm::vec4>("Vec4",
                                      sol::call_constructor, sol::constructors<glm::vec4(), glm::vec4(float), glm::vec4(float, float, float, float), glm::vec4(glm::vec4)>(),
                                      "x", &glm::vec4::x,
                                      "y", &glm::vec4::y,
                                      "z", &glm::vec4::z,
                                      "w", &glm::vec4::w);

        eMath.new_usertype<glm::vec3>("Vec3",
                                      sol::call_constructor, sol::constructors<glm::vec3(), glm::vec3(float), glm::vec3(float, float, float), glm::vec3(glm::vec3)>(),
                                      "x", &glm::vec3::x,
                                      "y", &glm::vec3::y,
                                      "z", &glm::vec3::z);

        eMath.new_usertype<glm::vec2>("Vec2",
                                      sol::call_constructor, sol::constructors<glm::vec2(), glm::vec2(float), glm::vec2(float, float), glm::vec2(glm::vec2)>(),
                                      "x", &glm::vec2::x,
                                      "y", &glm::vec2::y);

        eMath.set_function("degrees", sol::overload(
                    [](const glm::vec2& angles){ return glm::degrees(angles);},
                    [](const glm::vec3& angles){ return glm::degrees(angles);}
                ));
        eMath.set_function("radians", sol::overload(
                    [](const glm::vec2& angles){ return glm::radians(angles); },
                    [](const glm::vec3& angles){ return glm::radians(angles); }
                ));
    }

}