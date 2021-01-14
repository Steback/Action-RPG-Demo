cmake_minimum_required(VERSION 3.17)
project(Prototype_Action_RPG)

set(CMAKE_CXX_STANDARD 20)

# Vulkan Hpp
include_directories($ENV{VULKAN_SDK}/include)
file(GLOB_RECURSE VULKAN $ENV{LD_LIBRARY_PATH}/libvulkan.so)
file(GLOB_RECURSE VALIDATION_LAYERS $ENV{LD_LIBRARY_PATH}/libVkLayer_*.so)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/glfw)

# glm
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/glm)

# imgui
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/imgui)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/imgui)

# spdlog
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/spdlog)

### SOURCE CODE
# Source code
file(GLOB_RECURSE RENDERER_HEADER_FILES source/core/renderer/*.hpp)
file(GLOB_RECURSE RENDERER_SOURCE_FILES source/core/renderer/*.cpp)

file(GLOB_RECURSE WINDOW_HEADER_FILES source/core/window/*.hpp)
file(GLOB_RECURSE WINDOW_SOURCE_FILES source/core/window/*.cpp)

file(GLOB_RECURSE LOGGER_HEADER_FILES source/core/logger/*.hpp)
file(GLOB_RECURSE LOGGER_SOURCE_FILES source/core/logger/*.cpp)

add_executable(Prototype_Action_RPG source/main.cpp
        ${RENDERER_SOURCE_FILES} ${RENDERER_HEADER_FILES}
        ${WINDOW_SOURCE_FILES} ${WINDOW_HEADER_FILES}
        ${LOGGER_SOURCE_FILES} ${LOGGER_HEADER_FILES})

target_link_libraries(${PROJECT_NAME}
        ${VULKAN}
        glfw
        glm
        imgui
        spdlog)