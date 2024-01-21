cmake_minimum_required(VERSION 3.0)

project(a1)
set(CMAKE_CXX_STANDARD 11)

find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_library(a1 src/hw.cpp src/sw.cpp deps/src/gl.c)
target_include_directories(a1 PUBLIC /opt/homebrew/include)
target_include_directories(a1 PUBLIC deps/include)
target_link_libraries(a1 glm::glm OpenGL::GL SDL2::SDL2)

add_executable(e1 examples/e1.cpp)
target_link_libraries(e1 a1)

add_executable(e2 examples/e2.cpp)
target_link_libraries(e2 a1)

add_executable(e3 examples/e3.cpp)
target_link_libraries(e3 a1)

add_executable(e4 examples/e4.cpp)
target_link_libraries(e4 a1)

add_executable(e5 examples/e5.cpp)
target_link_libraries(e5 a1)
