set(SOURCE_PATH "/usr/local/share/gcf")
vcpkg_cmake_configure(SOURCE_PATH ${SOURCE_PATH} DISABLE_PARALLEL_CONFIGURE
                      OPTIONS -DBUILD_TESTING=OFF)
vcpkg_cmake_install(ADD_BIN_TO_PATH)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
# vcpkg_fixup_cmake_targets is marked as deprecated and vcpkg_cmake_config_fixup
# is listed as its replacement. However, for our purposes, it is not a drop in
# replacement and additional work needs to be done if we are to switch.
vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake TARGET_PATH share)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)
file(
    INSTALL ${SOURCE_PATH}/LICENSE
    DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT}
    RENAME copyright)

vcpkg_copy_pdbs()
