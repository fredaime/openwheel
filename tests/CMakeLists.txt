if(BUILD_DAEMON)
    add_executable(test_report
        ../openwheel-daemon/test_report.c
        ../openwheel-daemon/report.c
    )
    target_compile_features(test_report PRIVATE c_std_11)
    target_compile_options(test_report PRIVATE -Wall -Wextra)
    add_test(NAME hid_report COMMAND test_report)
    add_test(NAME daemon_executable_exists
        COMMAND ${CMAKE_COMMAND} -DTARGET_FILE=$<TARGET_FILE:asus_wheel>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/check_artifact.cmake)
endif()

if(BUILD_GADGET)
    find_package(Qt6 REQUIRED COMPONENTS Core Test)
    add_executable(test_rotation test_rotation.cpp
        ../openwheel-gadget/src/core/rotationhandler.cpp
        ../openwheel-gadget/src/profiles/profile.cpp
    )
    set_target_properties(test_rotation PROPERTIES AUTOMOC ON)
    target_include_directories(test_rotation PRIVATE ../openwheel-gadget/src)
    target_link_libraries(test_rotation PRIVATE Qt6::Core Qt6::Test)
    add_test(NAME test_rotation COMMAND test_rotation)
    add_test(NAME gadget_executable_exists
        COMMAND ${CMAKE_COMMAND} -DTARGET_FILE=$<TARGET_FILE:openwheel-gadget>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/check_artifact.cmake)
endif()
