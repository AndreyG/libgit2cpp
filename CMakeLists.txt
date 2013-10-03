project (libgit2cpp)
cmake_minimum_required(VERSION 2.8)

include_directories(src)

set(CMAKE_CXX_FLAGS -std=c++11)

set(lib_sources
    src/git/diff_list.cpp
    src/git/index.cpp
    src/git/repo.cpp
    src/git/threads_initializer.cpp
)

add_library(libgit2cpp ${lib_sources})
target_link_libraries(libgit2cpp git2)

add_executable(add src/add.cpp)
add_executable(log src/log.cpp)
add_executable(branch src/branch.cpp)

target_link_libraries(add       libgit2cpp)
target_link_libraries(log       libgit2cpp)
target_link_libraries(branch    libgit2cpp)
