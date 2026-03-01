include(${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake)

## Sources
set(SOURCES
    src/stb.cpp
    src/glad.cpp

    src/meta.cpp
    src/enum/surface.cpp
    src/enum/meta.cpp
    src/camera.old.cpp
    src/datatypes/vector.cpp
    src/datatypes/cframe.cpp
    src/datatypes/signal.cpp
    src/datatypes/enum.cpp
    src/datatypes/variant.cpp
    src/datatypes/color3.cpp
    src/datatypes/primitives.cpp
    src/datatypes/ref.cpp
    src/common.cpp
    src/math_helper.cpp
    src/rendering/assets.cpp
    src/rendering/skybox.cpp
    src/rendering/mesh2d.cpp
    src/rendering/torus.cpp
    src/rendering/font.cpp
    src/rendering/debug/debugrenderer.cpp
    src/rendering/defaultmeshes.cpp
    src/rendering/renderer.cpp
    src/rendering/texture.cpp
    src/rendering/shader.cpp
    src/rendering/mesh.cpp
    src/rendering/texture3d.cpp
    src/rendering/frustum.cpp
    src/physics/world.cpp
    src/logger.cpp
    src/objects/service/jointsservice.cpp
    src/objects/service/script/serverscriptservice.cpp
    src/objects/service/script/scriptcontext.cpp
    src/objects/service/workspace.cpp
    src/objects/service/selection.cpp
    src/objects/datamodel.cpp
    src/objects/joint/weld.cpp
    src/objects/joint/jointinstance.cpp
    src/objects/joint/rotate.cpp
    src/objects/joint/snap.cpp
    src/objects/joint/rotatev.cpp
    src/objects/joint/motor6d.cpp
    src/objects/base/service.cpp
    src/objects/base/instance.cpp
    src/objects/pvinstance.cpp
    src/objects/hint.cpp
    src/objects/folder.cpp
    src/objects/model.cpp
    src/objects/part/clickdetector.cpp
    src/objects/part/part.cpp
    src/objects/part/basepart.cpp
    src/objects/part/wedgepart.cpp
    src/objects/message.cpp
    src/objects/script.cpp
    src/partassembly.cpp
    src/panic.cpp
    src/handles.cpp
    src/platform.cpp
    src/lua/enum.cpp
    src/lua/instancelib.cpp
    src/timeutil.cpp
)

set(AUTOGEN_SOURCES
    # Enum
    src/enum/part.h
    src/enum/surface.h
    # Data types
    src/datatypes/enum.h
    src/datatypes/cframe.h
    src/datatypes/vector.h
    src/datatypes/color3.h
)

### Autogen

# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Custom%20Commands.html
foreach (SRC ${AUTOGEN_SOURCES})
    string(REGEX REPLACE "[.]h$" ".cpp" OUT_SRC_NAME ${SRC})
    set(SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${SRC}")
    set(OUT_PATH "${CMAKE_BINARY_DIR}/generated/${OUT_SRC_NAME}")

    add_custom_command(
        OUTPUT "${OUT_PATH}"
        DEPENDS autogen
        DEPENDS "${SRC_PATH}"
        COMMAND "$<TARGET_FILE:autogen>" "${CMAKE_CURRENT_SOURCE_DIR}/src" "${SRC_PATH}" "${OUT_PATH}"
    )

    list(APPEND AUTOGEN_OUTS "${OUT_PATH}")
endforeach()

### /Autogen

# Add version info into the build
include(gitversion)

add_custom_target(autogen_build ALL
    DEPENDS ${AUTOGEN_OUTS}
)

list(APPEND SOURCES ${AUTOGEN_OUTS})
list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/src/version.cpp)
add_library(openblocks STATIC ${SOURCES})
set_target_properties(openblocks PROPERTIES OUTPUT_NAME "openblocks")
target_link_directories(openblocks PUBLIC ${LUAJIT_LIBRARY_DIRS})
target_link_libraries(openblocks Jolt pugixml::pugixml Freetype::Freetype glm::glm libluajit ${LuaJIT_LIBRARIES})
target_include_directories(openblocks PUBLIC "src" PRIVATE "${CMAKE_SOURCE_DIR}/external/glad" ${LUAJIT_INCLUDE_DIRS} ${stb_SOURCE_DIR})
add_dependencies(openblocks autogen_build autogen)

# Windows-specific dependencies
if(WIN32)
    target_link_libraries(openblocks shell32.lib)
endif()

install(TARGETS openblocks
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets"
        DESTINATION share/openblocks)