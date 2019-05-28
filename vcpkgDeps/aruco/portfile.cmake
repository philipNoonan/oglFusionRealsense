include(vcpkg_common_functions)

vcpkg_download_distfile(ARCHIVE
    URLS "https://sourceforge.net/projects/aruco/files/3.1.2/aruco-3.1.2.zip"
    FILENAME "aruco-3.1.2.zip"
    SHA512 5cba6647c61a1b79ec26d491f7980d158fe2df153e02d607fbd0f6c17787e4a4fa305a60e1202ace3133a2948b5d60dbd11a0c42a8543694c4748eb3d66927ae
)


vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${ARCHIVE}
	PATCHES
		0001_update_to_opencv4.patch
		#0002_windows_fix.patch
		#0003_and_or_fix.patch
		#0004_windows_export_symbols.patch
		#0005_debug_vector_size_fix.patch
		0006_remove_cout.patch
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
)

vcpkg_install_cmake()
vcpkg_copy_pdbs()

vcpkg_fixup_cmake_targets(CONFIG_PATH share/aruco)




file(GLOB EXEFILES_RELEASE ${CURRENT_PACKAGES_DIR}/bin/*.exe)
file(GLOB EXEFILES_DEBUG ${CURRENT_PACKAGES_DIR}/debug/bin/*.exe)
file(COPY ${EXEFILES_RELEASE} DESTINATION ${CURRENT_PACKAGES_DIR}/tools/aruco)
file(REMOVE ${EXEFILES_RELEASE} ${EXEFILES_DEBUG})

vcpkg_copy_tool_dependencies(${CURRENT_PACKAGES_DIR}/tools/aruco)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/aruco RENAME copyright)
