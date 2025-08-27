include(${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake)

## Sources
set(SOURCES
    src/stb.cpp
    src/glad.cpp

    src/ptr_helpers.h
    src/enum/part.h
    src/enum/surface.cpp
    src/enum/meta.h
    src/enum/annotation.h
    src/enum/surface.h
    src/camera.cpp
    src/datatypes/vector.cpp
    src/datatypes/variant.h
    src/datatypes/cframe.cpp
    src/datatypes/signal.cpp
    src/datatypes/base.h
    src/datatypes/enum.h
    src/datatypes/enum.cpp
    src/datatypes/primitives.h
    src/datatypes/cframe.h
    src/datatypes/variant.cpp
    src/datatypes/vector.h
    src/datatypes/color3.h
    src/datatypes/annotation.h
    src/datatypes/color3.cpp
    src/datatypes/primitives.cpp
    src/datatypes/ref.cpp
    src/datatypes/ref.h
    src/datatypes/signal.h
    src/common.cpp
    src/utils.h
    src/platform.h
    src/math_helper.cpp
    src/rendering/skybox.cpp
    src/rendering/mesh2d.cpp
    src/rendering/torus.cpp
    src/rendering/mesh.h
    src/rendering/font.cpp
    src/rendering/debug/debugrenderer.cpp
    src/rendering/texture.h
    src/rendering/shader.h
    src/rendering/defaultmeshes.cpp
    src/rendering/skybox.h
    src/rendering/mesh2d.h
    src/rendering/light.h
    src/rendering/renderer.cpp
    src/rendering/texture3d.h
    src/rendering/texture.cpp
    src/rendering/renderer.h
    src/rendering/shader.cpp
    src/rendering/mesh.cpp
    src/rendering/material.h
    src/rendering/torus.h
    src/rendering/font.h
    src/rendering/defaultmeshes.h
    src/rendering/texture3d.cpp
    src/physics/world.h
    src/physics/world.cpp
    src/logger.cpp
    src/handles.h
    src/timeutil.h
    src/error/error.h
    src/error/result.h
    src/error/instance.h
    src/error/data.h
    src/partassembly.h
    src/objects/service/jointsservice.cpp
    src/objects/service/script/serverscriptservice.h
    src/objects/service/script/serverscriptservice.cpp
    src/objects/service/script/scriptcontext.h
    src/objects/service/script/scriptcontext.cpp
    src/objects/service/workspace.cpp
    src/objects/service/selection.cpp
    src/objects/service/selection.h
    src/objects/service/jointsservice.h
    src/objects/service/workspace.h
    src/objects/datamodel.cpp
    src/objects/script.h
    src/objects/joint/snap.h
    src/objects/joint/jointinstance.h
    src/objects/joint/rotatev.h
    src/objects/joint/weld.cpp
    src/objects/joint/jointinstance.cpp
    src/objects/joint/rotate.cpp
    src/objects/joint/rotate.h
    src/objects/joint/weld.h
    src/objects/joint/snap.cpp
    src/objects/joint/rotatev.cpp
    src/objects/base/service.h
    src/objects/base/member.h
    src/objects/base/instance.h
    src/objects/base/service.cpp
    src/objects/base/instance.cpp
    src/objects/base/refstate.h
    src/objects/message.h
    src/objects/pvinstance.cpp
    src/objects/hint.cpp
    src/objects/pvinstance.h
    src/objects/base.h
    src/objects/folder.cpp
    src/objects/model.cpp
    src/objects/datamodel.h
    src/objects/folder.h
    src/objects/meta.cpp
    src/objects/model.h
    src/objects/part/part.cpp
    src/objects/part/part.h
    src/objects/part/wedgepart.h
    src/objects/part/basepart.cpp
    src/objects/part/wedgepart.cpp
    src/objects/part/basepart.h
    src/objects/meta.h
    src/objects/hint.h
    src/objects/annotation.h
    src/objects/message.cpp
    src/objects/script.cpp
    src/partassembly.cpp
    src/panic.cpp
    src/logger.h
    src/camera.h
    src/handles.cpp
    src/version.h
    src/common.h
    src/platform.cpp
    src/panic.h
    src/lua/instancelib.cpp
    src/timeutil.cpp
    src/luaapis.h
    src/math_helper.h
)

set(AUTOGEN_SOURCES
    # Objects
    src/objects/service/script/serverscriptservice.h
    src/objects/service/script/scriptcontext.h
    src/objects/service/selection.h
    src/objects/service/jointsservice.h
    src/objects/service/workspace.h
    src/objects/script.h
    src/objects/joint/snap.h
    src/objects/joint/jointinstance.h
    src/objects/joint/rotatev.h
    src/objects/joint/rotate.h
    src/objects/joint/weld.h
    src/objects/message.h
    src/objects/pvinstance.h
    src/objects/base.h
    src/objects/datamodel.h
    src/objects/folder.h
    src/objects/model.h
    src/objects/part/part.h
    src/objects/part/wedgepart.h
    src/objects/part/basepart.h
    src/objects/meta.h
    src/objects/hint.h
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
target_include_directories(openblocks PUBLIC "src" "../include" "${CMAKE_SOURCE_DIR}/external/glad" ${LUAJIT_INCLUDE_DIRS} ${stb_SOURCE_DIR})
add_dependencies(openblocks autogen_build autogen)

# Windows-specific dependencies
if(WIN32)
    target_link_libraries(openblocks shell32.lib)
endif()