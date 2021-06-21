cmake_minimum_required(VERSION 3.17)
project(Action_RPG_Demo)

set(CMAKE_CXX_STANDARD 20)

# Binary output
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(NO_OUTPUT_DIRS)

# Source code and link
add_subdirectory(source/engine)
add_subdirectory(source/editor)
add_subdirectory(source/game)

target_link_libraries(Editor core ${CONAN_LIBS} ${VALIDATION_LAYERS})
target_link_libraries(Game core ${CONAN_LIBS} ${VALIDATION_LAYERS})