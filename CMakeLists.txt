project (libgit2cpp)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS -std=c++11)

file(GLOB lib_sources
    src/*.cpp
    include/git2cpp/*.h
)

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
