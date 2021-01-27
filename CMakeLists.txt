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
add_subdirectory(source/core)

add_executable(Prototype_Action_RPG source/main.cpp)
target_link_libraries(${PROJECT_NAME}
        ${VULKAN}
        glfw
        glm
        imgui
        spdlog
        core)

### COMPILE SHADERS ###
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/shaders)

set(GLSL_VALIDATOR $ENV{VULKAN_SDK}/bin/glslc)
file(GLOB_RECURSE shaders_source assets/shaders/*.vert assets/shaders/*.frag)

foreach(shader ${shaders_source})
    get_filename_component(fileName ${shader} NAME_WE)
    get_filename_component(fileExt ${shader} EXT)
    message("Compiling shader: ${fileName}${fileExt}.spv")
    execute_process(COMMAND ${GLSL_VALIDATOR} ${shader} -o ${fileName}${fileExt}.spv
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/shaders)
endforeach()