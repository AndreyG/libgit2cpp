project (git2cpp)
cmake_minimum_required(VERSION 2.8)

include_directories(include)

set(CMAKE_CXX_FLAGS -std=c++11)

set(lib_sources
    src/diff_list.cpp
    src/id_to_str.cpp
    src/index.cpp
    src/object.cpp
    src/odb.cpp
    src/odb_object.cpp
    src/repo.cpp
    src/revspec.cpp
    src/revwalker.cpp
    src/threads_initializer.cpp
)

add_library(git2cpp ${lib_sources})
target_link_libraries(git2cpp git2)

add_executable(add      examples/add.cpp)
add_executable(log      examples/log.cpp)
add_executable(branch   examples/branch.cpp)
add_executable(cat-file examples/cat-file.cpp)
add_executable(diff     examples/diff.cpp)

target_link_libraries(add       git2cpp)
target_link_libraries(log       git2cpp)
target_link_libraries(branch    git2cpp)
target_link_libraries(cat-file  git2cpp)
target_link_libraries(diff      git2cpp)
