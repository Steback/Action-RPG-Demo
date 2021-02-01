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

# imgui
include_directories(bindings)
file(GLOB_RECURSE IMGUI_BINDINGS bindings/*.cpp)

### SOURCE CODE
add_subdirectory(source/core)

add_executable(Prototype_Action_RPG source/main.cpp ${IMGUI_BINDINGS})
target_link_libraries(Prototype_Action_RPG
        core
        ${CONAN_LIBS}
        ${VALIDATION_LAYERS})

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_link_libraries(Prototype_Action_RPG ${VALIDATION_LAYERS})
endif()

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