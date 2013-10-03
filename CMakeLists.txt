project (libgit2cpp)
cmake_minimum_required(VERSION 2.8)

include_directories(include)

set(CMAKE_CXX_FLAGS -std=c++11)

set(lib_sources
    src/diff_list.cpp
    src/index.cpp
    src/repo.cpp
    src/threads_initializer.cpp
)

add_library(libgit2cpp ${lib_sources})
target_link_libraries(libgit2cpp git2)

add_executable(add      examples/add.cpp)
add_executable(log      examples/log.cpp)
add_executable(branch   examples/branch.cpp)

target_link_libraries(add       libgit2cpp)
target_link_libraries(log       libgit2cpp)
target_link_libraries(branch    libgit2cpp)
