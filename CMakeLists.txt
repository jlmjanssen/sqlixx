# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
# SPDX-License-Identifier: BSL-1.0

cmake_minimum_required(VERSION 3.31)

include(cmake/enable-experimental-import-std.cmake)

project(sqlixx VERSION 1.0.0 LANGUAGES C CXX)

include(cmake/enable-clang-tidy.cmake)
include(cmake/enable-coverage.cmake)
include(cmake/enable-sanitizer.cmake)

include(cmake/get_cpm.cmake)
include(cmake/fetch-project-dependencies.cmake)

# -- Build ---------------------------------------------------------------------

add_library(sqlixx.sqlixx STATIC)
add_library(sqlixx::sqlixx ALIAS sqlixx.sqlixx)

target_compile_features(sqlixx.sqlixx PUBLIC cxx_std_23)

set_target_properties(
    sqlixx.sqlixx
    PROPERTIES CXX_EXTENSIONS OFF CXX_MODULE_STD ON OUTPUT_NAME sqlixx
)

target_sources(
    sqlixx.sqlixx
    PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(sqlixx.sqlixx PUBLIC sqlite::sqlite)

target_enable_clang_tidy(sqlixx.sqlixx)
target_enable_coverage(sqlixx.sqlixx)
target_enable_sanitizer(sqlixx.sqlixx)

add_subdirectory(src)

# -- Tests ---------------------------------------------------------------------

option(
    SQLIXX_BUILD_TESTING
    "Build unit tests for SQLixx"
    ${PROJECT_IS_TOP_LEVEL}
)

if(SQLIXX_BUILD_TESTING)
    enable_testing()
    include(cmake/fetch-test-dependencies.cmake)
    add_subdirectory(tests)
endif()

# -- Install & Export ----------------------------------------------------------

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(ConfigPackageLocation "${CMAKE_INSTALL_LIBDIR}/cmake/sqlixx")

install(
    TARGETS sqlixx.sqlixx
    EXPORT sqlixx-targets
    FILE_SET CXX_MODULES DESTINATION "${ConfigPackageLocation}/src"
)

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/sqlixx-config.cmake"
    INSTALL_DESTINATION "${ConfigPackageLocation}"
    NO_SET_AND_CHECK_MACRO
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/sqlixx-config-version.cmake"
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/sqlixx-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/sqlixx-config-version.cmake"
    DESTINATION "${ConfigPackageLocation}"
)

install(
    EXPORT sqlixx-targets
    DESTINATION "${ConfigPackageLocation}"
    NAMESPACE sqlixx::
    CXX_MODULES_DIRECTORY
    .
)

# -- Packaging -----------------------------------------------------------------

if(PROJECT_IS_TOP_LEVEL)
    include(InstallRequiredSystemLibraries)
    include(CPack)
endif()
