# Third party
include_directories(../../thirdparty/glfw/include)
include_directories(../../thirdparty/glm)
include_directories(../../thirdparty/spdlog/include)
include_directories(../../thirdparty/imgui)

# Source Code
file(GLOB_RECURSE RENDERER_HEADER_FILES renderer/*.hpp)
file(GLOB_RECURSE RENDERER_SOURCE_FILES renderer/*.cpp)

file(GLOB_RECURSE WINDOW_HEADER_FILES window/*.hpp)
file(GLOB_RECURSE WINDOW_SOURCE_FILES window/*.cpp)

file(GLOB_RECURSE LOGGER_HEADER_FILES logger/*.hpp)
file(GLOB_RECURSE LOGGER_SOURCE_FILES logger/*.cpp)

file(GLOB_RECURSE MESH_HEADER_FILES model/*.hpp)
file(GLOB_RECURSE MESH_SOURCE_FILES model/*.cpp)

file(GLOB_RECURSE UI_HEADER_FILES ui/*.hpp)
file(GLOB_RECURSE UI_SOURCE_FILES ui/*.cpp)

file(GLOB_RECURSE CAMERA_HEADER_FILES camera/*.hpp)
file(GLOB_RECURSE CAMERA_SOURCE_FILES camera/*.cpp)

add_library(core STATIC
        ${CAMERA_SOURCE_FILES} ${CAMERA_HEADER_FILES}
        ${UI_SOURCE_FILES} ${UI_HEADER_FILES}
        ${MESH_SOURCE_FILES} ${MESH_HEADER_FILES}
        ${RENDERER_SOURCE_FILES} ${RENDERER_HEADER_FILES}
        ${WINDOW_SOURCE_FILES} ${WINDOW_HEADER_FILES}
        ${LOGGER_SOURCE_FILES} ${LOGGER_HEADER_FILES})