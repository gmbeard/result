cmake_minimum_required(VERSION 3.7)

project(Result)

list(APPEND
    CMAKE_PREFIX_PATH
    ${CMAKE_CURRENT_LIST_DIR}/deps
)

list(APPEND
    CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}/submodules/cmake
)

include(InstallExternals)

add_subdirectory(include)

set(RESULT_ENABLE_TESTING
    OFF
    CACHE
    BOOL
    "Enable unit tests for ${PROJECT_NAME}"
)

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
