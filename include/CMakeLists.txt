find_package(Cncpts REQUIRED)

add_library(result INTERFACE)

target_include_directories(
    result
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(
    result
    INTERFACE
        cxx_decltype_auto
)

target_compile_options(
    result
    INTERFACE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror -Wextra>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /wd4297>
)

target_link_libraries(
    result
    INTERFACE
        Cncpts::cncpts
)

install(
    DIRECTORY
        result
    DESTINATION
        include
)

install(
    TARGETS
        result
    EXPORT
        ResultTargets
)

add_library(Result::result ALIAS result)
