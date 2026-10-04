cmake_minimum_required(VERSION 3.16)
project(openwheel-daemon C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS dbus-1)

if(NOT DBUS_FOUND)
    message(WARNING "DBus not found - daemon will not be built")
    return()
endif()

# Source files
set(DAEMON_SRCS
    hidreader.c
    helpers.c
    report.c
)

set(DAEMON_HEADERS
    openwheel.h
    helpers.h
    report.h
)

# Create executable
add_executable(asus_wheel ${DAEMON_SRCS} ${DAEMON_HEADERS})

if(BUILD_TESTS)
    add_executable(test_report test_report.c report.c)
    target_compile_options(test_report PRIVATE -Wall -Wextra)
    add_test(NAME hid_report COMMAND test_report)
endif()

# Include directories
target_include_directories(asus_wheel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DBUS_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(asus_wheel PRIVATE
    ${DBUS_LIBRARIES}
)

# Compile options
target_compile_options(asus_wheel PRIVATE
    ${DBUS_CFLAGS_OTHER}
    -Wall
    -Wextra
)

# Installation
install(TARGETS asus_wheel
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT daemon
)

# Install headers (for development)
install(FILES ${DAEMON_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openwheel
    COMPONENT daemon-dev
)
