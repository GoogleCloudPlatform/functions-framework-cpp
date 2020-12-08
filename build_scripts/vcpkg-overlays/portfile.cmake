vcpkg_from_github(
    OUT_SOURCE_PATH
    SOURCE_PATH
    REPO
    GoogleCloudPlatform/functions-framework-cpp
    REF
    5d3f0bf618e3b28071c3f1668cf8fe8496933f3a
    SHA512
    ae8d092b7266d010c013b502176b7d28edf0abc36715a68cc54bbc6618d7ec3eda68a66fabe0917e9cc0594c4e052ffaf79a7501eaf2ed075723863f07c226e7
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
