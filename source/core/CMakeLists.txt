cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 20)

# Third party
include_directories("../../thirdparty/glfw/include")
include_directories("../../thirdparty/spdlog/include")

# Source code
file(GLOB_RECURSE RENDERER_HEADER_FILES renderer/*.hpp)
file(GLOB_RECURSE RENDERER_SOURCE_FILES renderer/*.cpp)

file(GLOB_RECURSE WINDOW_HEADER_FILES window/*.hpp)
file(GLOB_RECURSE WINDOW_SOURCE_FILES window/*.cpp)

file(GLOB_RECURSE LOGGER_HEADER_FILES logger/*.hpp)
file(GLOB_RECURSE LOGGER_SOURCE_FILES logger/*.cpp)

add_library(prototype_core STATIC
        ${RENDERER_SOURCE_FILES} ${RENDERER_HEADER_FILES}
        ${WINDOW_SOURCE_FILES} ${WINDOW_HEADER_FILES}
        ${LOGGER_SOURCE_FILES} ${LOGGER_HEADER_FILES})