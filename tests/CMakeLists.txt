include(${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake)

add_executable(obtest
    src/common.cpp
    src/lua/luaenum.cpp
    src/lua/luageneric.cpp
    src/lua/luainst.cpp
    src/lua/luamethod.cpp
    src/lua/luasched.cpp
    src/lua/luasignal.cpp
    src/objectmodel/basic.cpp
    src/objectmodel/datamodel.cpp
    src/objectmodel/hierarchyutils.cpp
    src/objectmodelv2/typemeta.cpp
    src/objectmodelv2/members.cpp
    src/objectmodelv2/categories.cpp
    src/objectmodelv2/inheritance.cpp
)
target_link_libraries(obtest PRIVATE openblocks Catch2::Catch2WithMain)
target_include_directories(obtest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
catch_discover_tests(obtest)