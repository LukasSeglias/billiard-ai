# This function is used to force a build on a dependant project at cmake configuration phase.
# credit: https://stackoverflow.com/a/23570741/320103
function (build_external_project target prefix url branch) #FOLLOWING ARGUMENTS are the CMAKE_ARGS of ExternalProject_Add

    set(trigger_build_dir ${CMAKE_BINARY_DIR}/force_${target})

    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT "
        cmake_minimum_required(VERSION 3.0)
        project(ExternalProjectCustom)
        include(ExternalProject)
        ExternalProject_add(${target}
            PREFIX ${prefix}/${target}
            GIT_REPOSITORY  ${url}
            GIT_TAG ${branch}
            CMAKE_ARGS ${ARGN})
        add_custom_target(trigger_${target})
        add_dependencies(trigger_${target} ${target})
    ")

    file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")

    execute_process(COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR} ..
            WORKING_DIRECTORY ${trigger_build_dir}/build
            )
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
            WORKING_DIRECTORY ${trigger_build_dir}/build
            )

endfunction()