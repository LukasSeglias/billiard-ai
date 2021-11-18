project(billiard_capture)

option(BILLIARD_CAPTURE_WITH_EBUS_SDK "Enable EBUS SDK" OFF)

add_library(${PROJECT_NAME} SHARED)
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${OpenCV_INCLUDE_DIRS}
        )

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS} ${realsense2_LIBRARY})

if(BILLIARD_CAPTURE_WITH_EBUS_SDK)
    target_link_libraries(${PROJECT_NAME} PUBLIC camera_sdk)
endif()
