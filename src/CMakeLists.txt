cmake_minimum_required(VERSION 3.22.1)
project(tvid_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(OpenGL REQUIRED)

FetchContent_Declare(
        glfw3
        URL https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.zip
)

FetchContent_MakeAvailable(glfw3)

include_directories(imgui/sources)
include_directories(imgui/backend)

include_directories(stb_image)
add_library(stb_image STATIC stb_image/stb_image.cpp
                             stb_image/stb_image_write.cpp)

include_directories(imGuiFileDialog)
add_library(imGuiFileDialog STATIC imGuiFileDialog/ImGuiFileDialog.cpp)

add_library(imgui_lib STATIC)
target_link_directories(imgui_lib PUBLIC imgui/sources imgui/backend)
add_subdirectory(imgui)

add_executable(tvid_project main.cpp application.cpp)

target_link_libraries(tvid_project imgui_lib imGuiFileDialog ${OPENGL_LIBRARIES} glfw stb_image dl)