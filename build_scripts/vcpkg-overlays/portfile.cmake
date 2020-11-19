vcpkg_from_github(
    OUT_SOURCE_PATH
    SOURCE_PATH
    REPO
    coryan/functions-framework-cpp
    REF
    d31250bce4ae9d9b6e3a99fe5e794c5ca515e728
    SHA512
    a90e3bcd5fa2d4c9eeaec58261de23a7da947fed87e9f0f73cc7378d09cbd3b43c1ce454bf69517b703c51e3af2c15cda0afe92c2ae4d0bac7c6d9749826ca19
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
