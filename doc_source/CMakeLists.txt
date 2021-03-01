
add_custom_target(document
        COMMAND docker-compose up
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/doc_source)