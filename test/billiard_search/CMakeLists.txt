project(billiard_search_test)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_executable(${PROJECT_NAME})
add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        # Copy resources to binary folder
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources

        # Copy dependencies to binary folder
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:billiard_search> ${PROJECT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:billiard_physics> ${PROJECT_BINARY_DIR}/
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
        BILLIARD_SEARCH_IMPORT
        GTEST_LINKED_AS_SHARED_LIBRARY=1)

target_link_libraries(${PROJECT_NAME} PRIVATE billiard_search gtest gtest_main gmock gmock_main)
