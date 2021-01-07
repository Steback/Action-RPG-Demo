cmake_minimum_required(VERSION 3.17)
project(Prototype_Action_RPG)

set(CMAKE_CXX_STANDARD 20)

# Vulkan Hpp
include_directories($ENV{VULKAN_SDK}/include)
file(GLOB_RECURSE VULKAN $ENV{LD_LIBRARY_PATH}/libvulkan.so)

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

add_executable(Prototype_Action_RPG main.cpp)
target_link_libraries(${PROJECT_NAME}
        ${VULKAN}
        glfw
        glm
        imgui)