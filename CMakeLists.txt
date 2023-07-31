cmake_minimum_required(VERSION 3.12)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Find OpenGL package
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# Find X11 package
find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})

# Add ImGui source files
set(IMGUI_SRC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

include_directories(imgui)
include_directories(imgui/backends)

include_directories(IconFontCppHeaders)

add_executable(titlebar ${IMGUI_SRC} titlebar/main.cpp)
target_link_libraries(titlebar ${OPENGL_LIBRARIES} ${X11_LIBRARIES})
