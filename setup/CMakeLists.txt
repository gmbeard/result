cmake_minimum_required(VERSION 3.7)

include(ExternalProject)

find_package(Cncpts QUIET)
if(NOT Cncpts_FOUND)
    ExternalProject_Add(
        Cncpts
        GIT_REPOSITORY https://github.com/gmbeard/cncpts.git
        INSTALL_DIR ${INSTALL_DEPS_TO}
        CMAKE_ARGS
            -DCMAKE_PREFIX_PATH=<INSTALL_DIR>
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )
endif()
