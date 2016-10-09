project (libgit2cpp)
cmake_minimum_required(VERSION 2.8)

# Build options
OPTION( USE_BOOST       "Enable use of boost header libraries" OFF )


set(CMAKE_CXX_FLAGS -std=c++11)

file(GLOB_RECURSE lib_sources
    src/*.cpp
    include/git2cpp/*.h
)

IF(USE_BOOST)
    add_definitions(-DUSE_BOOST=1)
    find_package(Boost REQUIRED)
    include_directories(${BOOST_INCLUDEDIR})
ENDIF()

add_library(git2cpp ${lib_sources})

target_include_directories(git2cpp
  PRIVATE
    src
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(git2cpp git2)

install_targets(/lib git2cpp)

set(examples
    add
    branch
    cat-file
    diff
    log
    rev-list
    showindex
    status
    init
    rev-parse
    general
    commit-graph-generator
)

foreach (example ${examples})
    add_executable(${example} examples/${example}.cpp)
    target_link_libraries(${example} git2cpp)
endforeach(example)

file(COPY test.sh DESTINATION . FILE_PERMISSIONS ${EXE_PERM})
