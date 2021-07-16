include_directories(../engine)

file(GLOB_RECURSE GAME_HEADER_FILES *.hpp)
file(GLOB_RECURSE GAME_SOURCE_FILES *.cpp)

# ImGui
file(GLOB_RECURSE IMGUI_BINDINGS ../../bindings/*.cpp)
include_directories(../../bindings)

add_executable(Game main.cpp ${IMGUI_BINDINGS} ${GAME_SOURCE_FILES} ${GAME_HEADER_FILES})
target_link_libraries(Game core ${CONAN_LIBS})
