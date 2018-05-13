add_subdirectory(catch)

add_executable(
    result_tests
    main.cpp
    result_tests.cpp
)

target_link_libraries(
    result_tests
    PRIVATE
        Result::result
        Catch2::Catch
)

add_test(
    NAME ResultTests
    COMMAND result_tests -s
)
