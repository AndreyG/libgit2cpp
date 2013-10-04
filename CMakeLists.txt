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

set(examples
    add
    branch
    cat-file
    diff
    log
    rev-list
)

foreach (example ${examples})
    add_executable(${example} examples/${example}.cpp)
    target_link_libraries(${example} git2cpp)
endforeach(example)
