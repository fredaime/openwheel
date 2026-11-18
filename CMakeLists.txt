cmake_minimum_required(VERSION 3.16)
project(openwheel VERSION 0.1.0 LANGUAGES C CXX)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Project-wide options
option(BUILD_DAEMON "Build openwheel-daemon" ON)
option(BUILD_GADGET "Build openwheel-gadget" ON)
option(BUILD_TESTS "Build tests" ON)
option(ENABLE_INSTALL "Enable installation targets" ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include standard CMake modules
include(GNUInstallDirs)
if(BUILD_TESTS)
    enable_testing()
endif()

# Add subdirectories
if(BUILD_DAEMON)
    message(STATUS "Building openwheel-daemon")
    add_subdirectory(openwheel-daemon)
endif()

if(BUILD_GADGET)
    message(STATUS "Building openwheel-gadget")
    add_subdirectory(openwheel-gadget)
endif()

# Enable testing
if(BUILD_TESTS)
    add_subdirectory(tests)
    message(STATUS "Testing enabled")
endif()

# Installation summary
message(STATUS "")
message(STATUS "Build Configuration Summary:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build daemon: ${BUILD_DAEMON}")
message(STATUS "  Build gadget: ${BUILD_GADGET}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
