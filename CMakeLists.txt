project (libgit2cpp)
cmake_minimum_required(VERSION 2.8)

include_directories(include)

set(CMAKE_CXX_FLAGS -std=c++11)

file(GLOB lib_sources
    src/*.cpp
    include/git2cpp/*.h
)

add_library(git2cpp ${lib_sources})
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
)

foreach (example ${examples})
    add_executable(${example} examples/${example}.cpp)
    target_link_libraries(${example} git2cpp)
endforeach(example)
