cmake_minimum_required(VERSION 3.18.2)
project(billiard_ai_main)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(ext)
set(BUILD_SHARED_LIBS ON CACHE INTERNAL "")
find_package(OpenCV REQUIRED CONFIG
        PATHS ${CMAKE_BINARY_DIR}/ext/opencv/opencv)
unset(BUILD_SHARED_LIBS)

find_package(realsense2 REQUIRED CONFIG
        PATHS ${CMAKE_BINARY_DIR}/librealsense/lib/cmake/realsense2)

add_subdirectory(lib)
add_subdirectory(test)
add_subdirectory(doc_source)
