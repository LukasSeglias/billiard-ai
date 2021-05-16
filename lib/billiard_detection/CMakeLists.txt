project(billiard_detection)

add_library(${PROJECT_NAME} SHARED)
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${OpenCV_INCLUDE_DIRS}
        )

add_subdirectory(src)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        BILLIARD_CAPTURE_IMPORT)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS} glm billiard_capture)
