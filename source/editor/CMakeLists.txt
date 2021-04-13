include_directories(../engine)

file(GLOB_RECURSE EDITOR_SOURCE_FILES *.cpp)

# ImGui
file(GLOB_RECURSE IMGUI_BINDINGS ../../bindings/*.cpp)
include_directories(../../bindings)

add_executable(Editor main.cpp ${IMGUI_BINDINGS} ${EDITOR_SOURCE_FILES})
