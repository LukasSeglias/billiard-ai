
add_library(camera_sdk STATIC null.cpp)

set(CAMERA_SDK_LIBS)
set(CAMERA_SDK_INCLUDE_DIR)
set(CAMERA_SDK_FOUND FALSE)

if (WIN32)
    message(STATUS "camera_sdk - Win32 supported")
    add_subdirectory(win/x64)
else ()
endif ()

if (CAMERA_SDK_FOUND)
    target_include_directories(camera_sdk
            PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CAMERA_SDK_INCLUDE_DIR}>
            )
endif ()

target_link_libraries(camera_sdk PUBLIC ${CAMERA_SDK_LIBS})