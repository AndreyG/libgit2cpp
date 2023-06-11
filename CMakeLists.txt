project (libgit2cpp)
cmake_minimum_required(VERSION 3.5.1)

# Build options
OPTION(USE_BOOST "Enable use of boost header libraries" OFF)

IF(USE_BOOST)
  add_definitions(-DUSE_BOOST=1)
  find_package(Boost REQUIRED)
  include_directories(${BOOST_INCLUDEDIR})
ENDIF()

if (MSVC)
  OPTION(BUNDLE_LIBGIT2 "use bundled libgit2" ON)
else()
  OPTION(BUNDLE_LIBGIT2 "use bundled libgit2" OFF)
endif()

if (BUNDLE_LIBGIT2)
  add_subdirectory(libs/libgit2)
endif()

file(GLOB_RECURSE lib_sources
  src/*.cpp
  include/git2cpp/*.h
)

if (MSVC AND ($MSVC_VERSION VERSION_GREATER 1900))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif()

add_library(git2cpp STATIC ${lib_sources})

target_include_directories(git2cpp
  PRIVATE
    src
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if (USE_BOOST)
  target_include_directories(git2cpp PUBLIC ${Boost_INCLUDE_DIRS})
endif()

if (BUNDLE_LIBGIT2)
  target_include_directories(git2cpp
    PUBLIC libs/libgit2/include
  )
  target_link_libraries(git2cpp libgit2package)
else()
  target_link_libraries(git2cpp LibGit2::LibGit2)
endif()

set_target_properties(git2cpp PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  INTERFACE_COMPILE_FEATURES cxx_std_17
)

option(BUILD_LIBGIT2CPP_EXAMPLES ON)

if(BUILD_LIBGIT2CPP_EXAMPLES)
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
    remote
    checkout
    ls-files
    blame
    clone
  )
  
  foreach (example ${examples})
    add_executable("${example}-cpp" examples/${example}.cpp)
    target_link_libraries("${example}-cpp" git2cpp)
  endforeach(example)
  
  add_executable(commit-graph-generator examples/commit-graph-generator.cpp)
  target_link_libraries(commit-graph-generator git2cpp)
  
  file(COPY test.sh DESTINATION . FILE_PERMISSIONS ${EXE_PERM})
endif()
