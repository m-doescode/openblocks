vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO DanielChappuis/reactphysics3d
    REF "cd958bbc0c6e84a869388cba6613f10cc645b3cb"
    SHA512 9856c0e998473e0bfb97af9ced07952bbd4dfef79f7dc388b1ecf9b6c6406f7669333e441fe6cefdf40b32edc5a1b8e4cb35a8c15fccb64c28785aff5fd77113
    HEAD_REF master
    PATCHES "std_chrono.patch"
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME "reactphysics3d" CONFIG_PATH "lib/cmake/ReactPhysics3D")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")