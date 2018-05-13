cmake_minimum_required(VERSION 3.7)

project(Result)

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
        ResultExport
    NAMESPACE 
        Result::
    FILE 
        ResultTarget.cmake
    DESTINATION
        lib/cmake/result
)

install(
    FILES
        cmake/ResultConfig.cmake
    DESTINATION
        lib/cmake/result
)
