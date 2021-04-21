project(unity_adapter)

add_library(${PROJECT_NAME} SHARED)
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )

add_subdirectory(src)

add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD

        # Copy dependencies to unity folder
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:unity_adapter> ${UNITY_PLUGINS}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:billiard_search> ${UNITY_PLUGINS}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:billiard_capture> ${UNITY_PLUGINS}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:billiard_detection> ${UNITY_PLUGINS}/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:billiard_physics> ${UNITY_PLUGINS}/
        )

target_link_libraries(${PROJECT_NAME} PRIVATE billiard_search billiard_detection billiard_capture)