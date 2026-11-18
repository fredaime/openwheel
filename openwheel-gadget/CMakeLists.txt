cmake_minimum_required(VERSION 3.16)

project(openwheel-gadget VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickControls2 DBus)

# Try to find KDE Frameworks (optional for basic build)
find_package(ECM QUIET NO_MODULE)
if(ECM_FOUND)
    list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
    find_package(KF6 QUIET COMPONENTS Config CoreAddons I18n WindowSystem DBusAddons)
    if(KF6_FOUND)
        message(STATUS "KDE Frameworks 6 found")
    endif()
endif()

find_package(X11 QUIET COMPONENTS Xtst)

# Source files
set(openwheel_gadget_SRCS
    src/main.cpp
    src/core/dialcontroller.cpp
    src/core/rotationhandler.cpp
    src/core/actionexecutor.cpp
    src/profiles/profile.cpp
    src/profiles/profilemanager.cpp
    src/profiles/applicationmatcher.cpp
    src/dbus/dbusinterface.cpp
    src/ui/ui.qrc
)

# Create executable
add_executable(openwheel-gadget ${openwheel_gadget_SRCS})

target_include_directories(openwheel-gadget PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link Qt libraries
target_link_libraries(openwheel-gadget PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::DBus
)

# Link KDE Frameworks if available
if(KF6_FOUND)
    target_link_libraries(openwheel-gadget PRIVATE
        KF6::ConfigCore
        KF6::CoreAddons
        KF6::I18n
        KF6::WindowSystem
        KF6::DBusAddons
    )
else()
    message(WARNING "Building without KDE Frameworks - some features will be limited")
    # Add a define so we can conditionally compile KF-dependent code
    target_compile_definitions(openwheel-gadget PRIVATE NO_KDE_FRAMEWORKS)
endif()

# Link X11 libraries if available
if(X11_FOUND)
    target_compile_definitions(openwheel-gadget PRIVATE HAVE_X11)
    target_link_libraries(openwheel-gadget PRIVATE X11::X11 X11::Xtst)
else()
    message(STATUS "X11/XTest not found: building without X11 input simulation")
endif()

if(NOT ENABLE_INSTALL)
    return()
endif()

# Installation
install(TARGETS openwheel-gadget
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT gadget
)

# Install profiles
install(DIRECTORY ${CMAKE_SOURCE_DIR}/profiles/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/openwheel/profiles
    COMPONENT data
    FILES_MATCHING PATTERN "*.json"
)

# Install desktop file if it exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openwheel-gadget.desktop)
    install(FILES openwheel-gadget.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
        COMPONENT data
    )
endif()

# Install icon if it exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openwheel-gadget.png)
    install(FILES openwheel-gadget.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps
        COMPONENT data
    )
endif()

# Install headers for development
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openwheel-gadget
    COMPONENT gadget-dev
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

message(STATUS "")
message(STATUS "Gadget Configuration Summary:")
message(STATUS "  Qt version: ${Qt6_VERSION}")
message(STATUS "  KDE Frameworks: ${KF6_FOUND}")
message(STATUS "  X11 support: ${X11_FOUND}")
message(STATUS "")
