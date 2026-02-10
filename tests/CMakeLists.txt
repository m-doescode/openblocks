include(${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake)

add_executable(obtest
    src/common.cpp
    src/lua/luasched.cpp
    src/lua/luasignal.cpp
    src/lua/luageneric.cpp
    src/lua/luainst.cpp
    src/objectmodel/basic.cpp
    src/objectmodel/datamodel.cpp
    src/objectmodelv2/members.cpp
)
target_link_libraries(obtest PRIVATE openblocks Catch2::Catch2WithMain)
target_include_directories(obtest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
catch_discover_tests(obtest)