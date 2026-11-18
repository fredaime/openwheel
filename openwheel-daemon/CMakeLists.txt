find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED IMPORTED_TARGET dbus-1)

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

add_executable(asus_wheel ${DAEMON_SRCS} ${DAEMON_HEADERS})
target_compile_features(asus_wheel PRIVATE c_std_11)
target_compile_options(asus_wheel PRIVATE -Wall -Wextra)
target_link_libraries(asus_wheel PRIVATE PkgConfig::DBUS)

if(ENABLE_INSTALL)
    install(TARGETS asus_wheel
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT daemon)
    install(FILES ${DAEMON_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openwheel COMPONENT daemon-dev)
endif()
