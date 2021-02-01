cmake_minimum_required(VERSION 3.17)
project(Prototype_Action_RPG)

set(CMAKE_CXX_STANDARD 20)

# Vulkan
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    if (${UNIX})
        file(GLOB_RECURSE VALIDATION_LAYERS $ENV{LD_LIBRARY_PATH}/libVkLayer_*.so)
    endif()
endif()

# Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# ImGui
include_directories(bindings)
file(GLOB_RECURSE IMGUI_BINDINGS bindings/*.cpp)

### SOURCE CODE
add_subdirectory(source/core)

add_executable(Prototype_Action_RPG source/main.cpp ${IMGUI_BINDINGS})
target_link_libraries(Prototype_Action_RPG core ${CONAN_LIBS} ${VALIDATION_LAYERS})

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_link_libraries(Prototype_Action_RPG ${VALIDATION_LAYERS})
endif()

### COMPILE SHADERS ###
set(SHADERS_COMPILE_SCRIPT ${PROJECT_SOURCE_DIR}/tools/shadersCompile.py)
execute_process(COMMAND python ${SHADERS_COMPILE_SCRIPT} ${PROJECT_BINARY_DIR})