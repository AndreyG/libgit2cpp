set(VERSION "0.0.0" CACHE STRING "libgit2cpp Version")
set(COMMIT_HASH "")
set(COMMIT_COUNT 0)

find_package(Git)

if (Git_FOUND AND VERSION STREQUAL "0.0.0")
    message(STATUS "No version defined, fetching one from git")

    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE COMMIT_COUNT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )

    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )

    set(VERSION "r${COMMIT_COUNT}.${COMMIT_HASH}")
endif ()

message(STATUS "Version: ${VERSION}")
string(REGEX REPLACE "([0-9]+\\.[0-9]+\\.[0-9]+(\\.[0-9])?)(.*)" "\\1" VERSION_FOR_CMAKE "${VERSION}")
message(STATUS "Version for CMake: ${VERSION_FOR_CMAKE}")

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h)
