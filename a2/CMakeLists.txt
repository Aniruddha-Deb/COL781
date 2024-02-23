cmake_minimum_required(VERSION 3.0)

project(a2)
set(CMAKE_CXX_STANDARD 11)

find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_library(viewer src/mesh.cpp src/hw.cpp src/viewer.cpp deps/src/gl.c)
target_include_directories(viewer PUBLIC /opt/homebrew/include)
target_include_directories(viewer PUBLIC deps/include)
target_link_libraries(viewer glm::glm OpenGL::GL SDL2::SDL2)

add_executable(example src/example.cpp)
add_executable(square src/square.cpp)
target_link_libraries(example viewer)
target_link_libraries(square viewer)