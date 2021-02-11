cmake_minimum_required(VERSION 3.17)
project(Prototype_Action_RPG)

set(CMAKE_CXX_STANDARD 20)

# Binary output
file(MAKE_DIRECTORY bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# Vulkan
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    if (${UNIX})
        file(GLOB_RECURSE VALIDATION_LAYERS $ENV{LD_LIBRARY_PATH}/libVkLayer_*.so)
    endif()
endif()

# Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(NO_OUTPUT_DIRS)

# ImGui
include_directories(bindings)
file(GLOB_RECURSE IMGUI_BINDINGS bindings/*.cpp)

# Source code and link
add_subdirectory(source/core)
add_subdirectory(source/editor)

target_link_libraries(Editor core ${CONAN_LIBS} ${VALIDATION_LAYERS})

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_link_libraries(Editor ${VALIDATION_LAYERS})
endif()

### COMPILE SHADERS ###
set(SHADERS_COMPILE_SCRIPT ${PROJECT_SOURCE_DIR}/tools/shadersCompile.py)
execute_process(COMMAND python ${SHADERS_COMPILE_SCRIPT} ${PROJECT_SOURCE_DIR}/bin)