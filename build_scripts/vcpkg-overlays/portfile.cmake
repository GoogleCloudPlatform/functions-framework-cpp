vcpkg_from_github(
    OUT_SOURCE_PATH
    SOURCE_PATH
    REPO
    GoogleCloudPlatform/functions-framework-cpp
    REF
    9b3a8aa80e9e9a5ed6f8793a972d8a9d22231b9b
    SHA512
    869d63232ea544ad49e40b8d515c72bf6ac8896d929122b149953ee2bf73f03ef0d3c32c1de66a37cc50d4c55fd5714875632a317ae296c5ee0a92f21e7fad82
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
