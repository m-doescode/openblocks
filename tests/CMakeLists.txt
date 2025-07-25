function (create_test TEST_NAME)
    set(TARGET_NAME test_${TEST_NAME})
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_libraries(${TARGET_NAME} PRIVATE openblocks)
    add_dependencies(${TARGET_NAME} openblocks)
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endfunction ()

create_test(lua src/luatest.cpp)
create_test(luasched src/luaschedtest.cpp)
create_test(luasignal src/luasignaltest.cpp)

# https://stackoverflow.com/a/36729074/16255372
add_custom_target(check ${CMAKE_CTEST_COMMAND} --output-on-failure WORKING_DIRECTORY ${CMAKE_BINARY_DIR})