include(${CMAKE_SOURCE_DIR}/cmake/build_external_project.cmake)

build_external_project(
        librealsense
        ${PROJECT_BINARY_DIR}/external
        https://github.com/IntelRealSense/librealsense
        v2.43.0
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_UNIT_TESTS:BOOL=OFF
        -DIMPORT_DEPTH_CAM_FW:BOOL=ON
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_SHARED_LIBS:BOOL=ON
        -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_BINARY_DIR}/librealsense
)
