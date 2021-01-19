vcpkg_from_github(
    OUT_SOURCE_PATH
    SOURCE_PATH
    REPO
    GoogleCloudPlatform/functions-framework-cpp
    REF
    v0.1.0
    SHA512
    2f9b9b8b44036f07a20e580b9a9096668fb0c1385bb65e9422c901ac65daaa87708b577d495eea6a81f0857629939689517bacc1db0b7e2a86f40259811ec02e
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
