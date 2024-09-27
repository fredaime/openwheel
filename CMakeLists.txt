cmake_minimum_required(VERSION 3.0)
project(asus_wheel C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)
include_directories(.)

add_executable(asus_wheel
    hidreader.c)


target_include_directories(asus_wheel PRIVATE ${DBUS_INCLUDE_DIRS})

target_link_libraries(asus_wheel PRIVATE ${DBUS_LIBRARIES})

target_compile_options(asus_wheel PRIVATE ${DBUS_CFLAGS_OTHER})