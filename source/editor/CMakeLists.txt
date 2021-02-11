include_directories(../core)
include_directories(../../bindings)

file(GLOB_RECURSE EDITOR_HEADER_FILES *.hpp)
file(GLOB_RECURSE EDITOR_SOURCE_FILES *.cpp)

add_executable(Editor main.cpp ${IMGUI_BINDINGS} ${EDITOR_SOURCE_FILES} ${EDITOR_HEADER_FILES})
