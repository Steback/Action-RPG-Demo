include_directories(../core)

file(GLOB_RECURSE EDITOR_HEADER_FILES *.hpp)
file(GLOB_RECURSE EDITOR_SOURCE_FILES *.cpp)

# ImGui
file(GLOB_RECURSE IMGUI_BINDINGS ../../bindings/*.cpp)
include_directories(../../bindings)

add_executable(Game main.cpp ${IMGUI_BINDINGS} ${EDITOR_SOURCE_FILES} ${EDITOR_HEADER_FILES})
