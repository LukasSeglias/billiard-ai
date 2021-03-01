include(FetchContent)
FetchContent_Declare(
        opencv_contrib
        GIT_REPOSITORY https://github.com/opencv/opencv_contrib.git
        GIT_TAG        4.4.0
)
FetchContent_Populate(opencv_contrib)

include(${CMAKE_SOURCE_DIR}/cmake/build_external_project.cmake)

build_external_project(
        opencv
        ${CMAKE_CURRENT_BINARY_DIR}/external
        https://github.com/opencv/opencv.git
        4.4.0
        -DBUILD_opencv_core=ON
        -DBUILD_opencv_highgui=ON
        -DBUILD_opencv_imgproc=ON
        -DBUILD_opencv_contrib=ON
        -DBUILD_DOCS:BOOL=FALSE
        -DBUILD_EXAMPLES:BOOL=FALSE
        -DBUILD_TESTS:BOOL=FALSE
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_SHARED_LIBS:BOOL=TRUE
        -DBUILD_NEW_PYTHON_SUPPORT:BOOL=OFF
        -DBUILD_WITH_DEBUG_INFO=OFF
        -DWITH_CUDA:BOOL=FALSE
        -DWITH_FFMPEG:BOOL=TRUE
        -DWITH_GSTREAMER:BOOL=TRUE
        -DWITH_MSMF:BOOL=FALSE
        -DWITH_IPP:BOOL=FALSE
        -DBUILD_PERF_TESTS:BOOL=FALSE
        -DBUILD_PNG:BOOL=ON
        -DBUILD_JPEG:BOOL=ON
        -DBUILD_WITH_STATIC_CRT:BOOL=OFF
        -DBUILD_FAT_JAVA_LIB=OFF
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/opencv
        -DOPENCV_EXTRA_MODULES_PATH=${opencv_contrib_SOURCE_DIR}/modules
)