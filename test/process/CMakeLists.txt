project(process_test)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_executable(${PROJECT_NAME})
add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD

        # Copy dependencies to binary folder
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest_main> ${PROJECT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest> ${PROJECT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gmock_main> ${PROJECT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gmock> ${PROJECT_BINARY_DIR}/
        )
add_subdirectory(src)

target_compile_definitions(${PROJECT_NAME} PRIVATE GTEST_LINKED_AS_SHARED_LIBRARY=1)

target_link_libraries(${PROJECT_NAME} PRIVATE process gtest gtest_main gmock gmock_main)
