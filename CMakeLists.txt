cmake_minimum_required(VERSION 3.5.1)

project(libgit2cpp LANGUAGES C CXX)
set(package git2cpp)

option(USE_BOOST "Enable use of Boost header libraries" OFF)
option(BUNDLE_LIBGIT2 "Use bundled libgit2" ${MSVC})
option(BUILD_LIBGIT2CPP_EXAMPLES "Build libgit2cpp examples" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Version)

if (USE_BOOST)
    find_package(Boost REQUIRED)
    add_definitions(-DUSE_BOOST=1)
    include_directories(${BOOST_INCLUDEDIR})
endif ()

if (BUNDLE_LIBGIT2)
    add_subdirectory(libs/libgit2)
endif ()

file(GLOB_RECURSE LIBGIT2CPP_SOURCES
        src/*.cpp
        include/git2cpp/*.h
)

add_library(${package} STATIC ${LIBGIT2CPP_SOURCES})

set_target_properties(${package} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        INTERFACE_COMPILE_FEATURES cxx_std_17
)

if (MSVC AND ($MSVC_VERSION VERSION_GREATER 1900))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif ()

target_include_directories(${package}
        PRIVATE src
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if (USE_BOOST)
    target_include_directories(${package} PUBLIC ${Boost_INCLUDE_DIRS})
endif ()

if (BUILD_LIBGIT2CPP_EXAMPLES)
    add_subdirectory(examples)
    file(COPY test.sh DESTINATION . FILE_PERMISSIONS ${EXE_PERM})
endif ()

if (BUNDLE_LIBGIT2)
    target_include_directories(${package} PUBLIC libs/libgit2/include)
    target_link_libraries(${package} libgit2package)
else ()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(LibGit2 REQUIRED libgit2)
    target_link_libraries(${package} ${LibGit2_LIBRARIES})
endif ()

include(InstallRules)
