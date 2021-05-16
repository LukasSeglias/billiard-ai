project(billiard_detection_test)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_executable(${PROJECT_NAME})
add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD

        # Copy resources to binary folder
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources

        # Copy dependencies to binary folder
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:billiard_detection> ${PROJECT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:billiard_snooker> ${PROJECT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:billiard_capture> ${PROJECT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${realsense2_LIBRARY}> ${PROJECT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest_main> ${PROJECT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest> ${PROJECT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gmock_main> ${PROJECT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gmock> ${PROJECT_BINARY_DIR}/
        )
foreach(OpenCV_LIB ${OpenCV_LIBS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copy $<TARGET_FILE:${OpenCV_LIB}> to $<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${OpenCV_LIB}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
endforeach()
add_subdirectory(src)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        BILLIARD_DETECTION_IMPORT
        BILLIARD_SNOOKER_IMPORT
        BILLIARD_CAPTURE_IMPORT
        GTEST_LINKED_AS_SHARED_LIBRARY=1)
target_link_libraries(${PROJECT_NAME} PRIVATE billiard_detection billiard_snooker billiard_capture gtest gtest_main gmock gmock_main)
