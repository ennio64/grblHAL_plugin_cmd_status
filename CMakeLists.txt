# Plugin cmd_status per grblHAL

add_library(cmd_status INTERFACE)

target_sources(cmd_status INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/plugin_cmd_status.c
)

target_include_directories(cmd_status INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

