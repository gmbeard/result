cmake_minimum_required(VERSION 3.7)

project(Result)

list(APPEND
    CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}/cmake
)

option(SKIP_SUPERBUILD
    "SuperBuild for ${PROJECT_NAME}"
    OFF
)

option(RESULT_ENABLE_TESTING
    "Enable unit tests for ${PROJECT_NAME}"
    OFF
)

if(NOT SKIP_SUPERBUILD)
    include(SuperBuild)
    return()
endif()

add_subdirectory(include)

if(RESULT_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

install(
    EXPORT 
        ResultTargets
    NAMESPACE 
        Result::
    FILE 
        ResultTargets.cmake
    DESTINATION
        lib/cmake/result
)

install(
    FILES
        cmake/ResultConfig.cmake
    DESTINATION
        lib/cmake/result
)
