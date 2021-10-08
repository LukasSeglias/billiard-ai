project(billiard_search)

add_library(${PROJECT_NAME} SHARED)
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PUBLIC glm PRIVATE process billiard_physics billiard_debug)
