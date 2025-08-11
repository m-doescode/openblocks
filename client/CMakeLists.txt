include(${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake)

add_executable(client "src/main.cpp")
target_link_libraries(client PRIVATE openblocks glfw)
add_dependencies(client openblocks)

if(WIN32)
    # Copy assets
    add_custom_command(
        TARGET client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:client>/assets)
endif()

set_target_properties(client PROPERTIES
    WIN32_EXECUTABLE ON
)

# https://stackoverflow.com/a/73899349/16255372
if (WIN32)
    # /ENTRY:mainCRTStartup keeps the same "main" function instead of requiring "WinMain"
    target_link_options(client PRIVATE "/ENTRY:mainCRTStartup")
endif()