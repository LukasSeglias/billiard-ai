include(FetchContent)
FetchContent_Declare(
        opencv_contrib
        GIT_REPOSITORY https://github.com/opencv/opencv_contrib.git
        GIT_TAG        4.5.2
)
set(BILLIARD_OPENCV_VERSION_SHARED_LIBRARIES "452")
FetchContent_Populate(opencv_contrib)

include(${CMAKE_SOURCE_DIR}/cmake/build_external_project.cmake)

build_external_project(
        opencv
        ${CMAKE_CURRENT_BINARY_DIR}/external
        https://github.com/opencv/opencv.git
        4.5.2
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

# Find out location of opencv_videoio_ffmpeg shared library in order for other targets to copy it next to the executable
# This sets the variables BILLIARD_OPENCV_FFMPEG_FILENAME and BILLIARD_OPENCV_FFMPEG_FILEPATH for later use.
if(WIN32)
    # Source: https://github.com/opencv/opencv file opencv/cmake/OpenCVDetectCXXCompiler.cmake
    if(DEFINED OpenCV_ARCH AND DEFINED OpenCV_RUNTIME)
        set(BILLIARD_OPENCV_ARCH ${OpenCV_ARCH})
        set(BILLIARD_OPENCV_RUNTIME ${OpenCV_RUNTIME})
    elseif(MSVC)
        if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
            set(BILLIARD_OPENCV_ARCH "x64")
        elseif("${CMAKE_GENERATOR_PLATFORM}" MATCHES "ARM64")
            set(BILLIARD_OPENCV_ARCH "ARM64")
        elseif("${CMAKE_GENERATOR}" MATCHES "ARM")
            set(BILLIARD_OPENCV_ARCH "ARM")
        elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
            set(BILLIARD_OPENCV_ARCH "x64")
        else()
            set(BILLIARD_OPENCV_ARCH x86)
        endif()

        if(MSVC_VERSION MATCHES "^192[0-9]$")
            set(BILLIARD_OPENCV_RUNTIME vc16)
        elseif(MSVC_VERSION MATCHES "^193[0-9]$")
            set(BILLIARD_OPENCV_RUNTIME vc17)
        else()
            message(WARNING "Billiard-AI does not recognize MSVC_VERSION \"${MSVC_VERSION}\". Cannot set BILLIARD_OPENCV_RUNTIME in order to copy any opencv-ffmpeg-dlls.")
        endif()
    else()
        message(WARNING "Billiard-AI does not recognize your Compiler. Cannot set BILLIARD_OPENCV_RUNTIME in order to copy any opencv-ffmpeg-dlls.")
    endif()

    if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
        set(BILLIARD_OPENCV_PLUGIN_ARCH "_64" CACHE STRING "")
    else()
        set(BILLIARD_OPENCV_PLUGIN_ARCH "" CACHE STRING "")
    endif()

    set(BILLIARD_OPENCV_FFMPEG_FILENAME "")
    set(BILLIARD_OPENCV_FFMPEG_FILENAME "opencv_videoio_ffmpeg${BILLIARD_OPENCV_VERSION_SHARED_LIBRARIES}${BILLIARD_OPENCV_PLUGIN_ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set(BILLIARD_OPENCV_FFMPEG_FILENAME "${BILLIARD_OPENCV_FFMPEG_FILENAME}" CACHE INTERNAL "Opencv videoio ffmpeg shared library filename")

    set(BILLIARD_OPENCV_FFMPEG_FILEPATH "")
    cmake_path(APPEND BILLIARD_OPENCV_FFMPEG_FILEPATH "${OpenCV_DIR}" "${BILLIARD_OPENCV_ARCH}" "${BILLIARD_OPENCV_RUNTIME}" "bin" "${BILLIARD_OPENCV_FFMPEG_FILENAME}")
    set(BILLIARD_OPENCV_FFMPEG_FILEPATH "${BILLIARD_OPENCV_FFMPEG_FILEPATH}" CACHE INTERNAL "Opencv videoio ffmpeg shared library path")
endif()