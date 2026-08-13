cmake_minimum_required(VERSION 3.24)

project(
    auveh
    VERSION 0.1.1
    DESCRIPTION "Auto-vectorization helpers"
    LANGUAGES CXX
)

add_library(auveh INTERFACE)
add_library(ltla::auveh ALIAS auveh)

target_compile_features(auveh INTERFACE cxx_std_17)

include(GNUInstallDirs)
target_include_directories(auveh INTERFACE 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ltla_auveh>"
)

# Building the test-related machinery, if we are compiling this library directly.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(AUVEH_TESTS "Build auveh's test suite." ON)
else()
    option(AUVEH_TESTS "Build auveh's test suite." OFF)
endif()

if(AUVEH_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif() 
endif()

# Installing for find_package.
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ltla_auveh)

install(TARGETS auveh 
    EXPORT auvehTargets)

install(EXPORT auvehTargets
    FILE ltla_auvehTargets.cmake
    NAMESPACE ltla::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_auveh)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/ltla_auvehConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_auveh)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ltla_auvehConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ltla_auvehConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ltla_auvehConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_auveh)
