project(opengl LANGUAGES CXX)

find_package(OpenGL REQUIRED)

add_library(opengl INTERFACE)

target_link_libraries (opengl INTERFACE opengl32)
