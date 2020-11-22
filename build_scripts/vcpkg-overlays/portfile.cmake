vcpkg_from_github(
    OUT_SOURCE_PATH
    SOURCE_PATH
    REPO
    GoogleCloudPlatform/functions-framework-cpp
    REF
    4a4a0a534b872b8ea09dd4f6f5d31afab9d02127
    SHA512
    eabb6f96912025ffca090ec8ec5cf7aaffc40bc3f327465dcaf9dd496f2157f32517055e48226917c23dcc0890ed2700dcf8c70822725dc198bca37bfe27dc14
    HEAD_REF
    main)

vcpkg_configure_cmake(SOURCE_PATH ${SOURCE_PATH} PREFER_NINJA
                      DISABLE_PARALLEL_CONFIGURE OPTIONS -DBUILD_TESTING=OFF)

vcpkg_install_cmake(ADD_BIN_TO_PATH)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake TARGET_PATH share)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)
file(
    INSTALL ${SOURCE_PATH}/LICENSE
    DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT}
    RENAME copyright)

vcpkg_copy_pdbs()
