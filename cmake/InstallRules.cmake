set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(
        DIRECTORY include/
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        COMPONENT git2cpp_Development
)

install(
        TARGETS ${package}
        EXPORT git2cppTargets
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

if (NOT DEFINED VERSION)
    set(VERSION "1.0.0")
endif ()

write_basic_package_version_file(
        "${package}ConfigVersion.cmake"
        VERSION ${VERSION}
        COMPATIBILITY SameMajorVersion
        ARCH_INDEPENDENT
)

set(
        git2cpp_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
        CACHE PATH "CMake package config location relative to the install prefix"
)

mark_as_advanced(git2cpp_INSTALL_CMAKEDIR)

install(
        FILES cmake/InstallConfig.cmake
        DESTINATION "${git2cpp_INSTALL_CMAKEDIR}"
        RENAME "${package}Config.cmake"
        COMPONENT git2cpp_Development
)

install(
        FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
        DESTINATION "${git2cpp_INSTALL_CMAKEDIR}"
        COMPONENT git2cpp_Development
)

install(
        EXPORT git2cppTargets
        NAMESPACE git2cpp::
        DESTINATION "${git2cpp_INSTALL_CMAKEDIR}"
        COMPONENT git2cpp_Development
)

if (PROJECT_IS_TOP_LEVEL)
    include(CPack)
endif ()
