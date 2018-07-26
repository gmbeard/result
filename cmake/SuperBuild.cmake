include(ExternalProject)

find_package(Cncpts QUIET)
if(NOT Cncpts_FOUND)
    ExternalProject_Add(
        CncptsProject
        GIT_REPOSITORY https://github.com/gmbeard/cncpts.git
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        CMAKE_ARGS
            -DCMAKE_PREFIX_PATH=<INSTALL_DIR>
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )
else()
    add_custom_target(CncptsProject)
endif()

find_package(Catch2 QUIET)
if(NOT Catch2_FOUND)
    ExternalProject_Add(
        Catch2Project
        GIT_REPOSITORY https://github.com/CatchOrg/Catch2.git
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        CMAKE_ARGS
            -DCMAKE_PREFIX_PATH=<INSTALL_DIR>
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )
else()
    add_custom_target(Catch2Project)
endif()

ExternalProject_Add(
    ResultProject
    DEPENDS
        Catch2Project
        CncptsProject
    SOURCE_DIR ${PROJECT_SOURCE_DIR}
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS
        -DSKIP_SUPERBUILD=ON
        -DCMAKE_PREFIX_PATH=<INSTALL_DIR>
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
set(SKIP_SUPERBUILD ON)
