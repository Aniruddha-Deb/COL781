cmake_minimum_required(VERSION 3.0)

project(a4)
set(CMAKE_CXX_STANDARD 11)

find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_library(col781 src/hw.cpp src/camera.cpp deps/src/gl.c)
target_include_directories(col781 PUBLIC /opt/homebrew/include)
target_include_directories(col781 PUBLIC deps/include)
target_link_libraries(col781 glm::glm OpenGL::GL SDL2::SDL2)

add_executable(animation src/animation.cpp)
target_link_libraries(animation col781)
