cmake_minimum_required(VERSION 3.17)
project(Prototype_Action_RPG)

set(CMAKE_CXX_STANDARD 20)

# Binary output
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(NO_OUTPUT_DIRS)

# Source code and link
add_subdirectory(source/core)
add_subdirectory(source/editor)
add_subdirectory(source/game)

target_link_libraries(Editor core ${CONAN_LIBS} ${VALIDATION_LAYERS})
target_link_libraries(Game core ${CONAN_LIBS} ${VALIDATION_LAYERS})

### COMPILE SHADERS ###
set(SHADERS_COMPILE_SCRIPT ${PROJECT_SOURCE_DIR}/scripts/shadersCompile.py)
execute_process(COMMAND python ${SHADERS_COMPILE_SCRIPT} ${PROJECT_SOURCE_DIR}/bin)