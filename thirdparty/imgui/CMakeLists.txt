file(GLOB_RECURSE IMGUI_HEADER_FILES *.h)
file(GLOB_RECURSE IMGUI_SOURCE_FILES *.cpp)

include_directories(../glfw/include)

add_library(imgui STATIC ${IMGUI_SOURCE_FILES} ${IMGUI_HEADER_FILES})