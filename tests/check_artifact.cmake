if(NOT EXISTS "${TARGET_FILE}" OR IS_DIRECTORY "${TARGET_FILE}")
    message(FATAL_ERROR "Expected a built executable at ${TARGET_FILE}")
endif()
