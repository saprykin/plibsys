# config file support for find_package(PLib). This needs to
# deal with two environments: (1) finding the build tree and (2)
# finding the install tree. plib-config.cmake detects which
# situation it is handing by looking at @PLIB_ROOT_DIR@. If
# this is an absolute path, it's in the build tree; otherwise,
# it's in the install tree (note that the whole install tree
# can be relocated).

include (PlatformDetect.cmake)

plib_detect_target_platform (PLIB_TARGET_PLATFORM)

# plib-config.cmake for the build tree.
set(PLIB_ROOT_DIR ${PROJECT_BINARY_DIR})
set(PLib_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM})
configure_file(plib-config.cmake.in ${PROJECT_BINARY_DIR}/plib-config.cmake @ONLY)
configure_file(plib-config-version.cmake.in ${PROJECT_BINARY_DIR}/plib-config-version.cmake @ONLY)
export (TARGETS plib plibstatic FILE ${PROJECT_BINARY_DIR}/plib-targets.cmake)

# plib-config.cmake for the install tree. It's installed in
# ${INSTALL_CMAKE_DIR} and @PLIB_ROOT_DIR@ is the relative
# path to the root from there (note that the whole install tree can
# be relocated).
set (INSTALL_CMAKE_DIR cmake)
set (PLIB_ROOT_DIR ..)

# @PLib_INCLUDE_DIRS@ is not used in the install tree; reset
# it to prevent the source and build paths appearing in the installed
# config files.
set (PLib_INCLUDE_DIRS IGNORE)
configure_file (plib-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/plib-config.cmake @ONLY)
configure_file (plib-config-version.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/plib-config-version.cmake @ONLY)
install (FILES
	${CMAKE_CURRENT_BINARY_DIR}/plib-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/plib-config-version.cmake
	DESTINATION ${INSTALL_CMAKE_DIR}
	COMPONENT Core
)

# Make information about the cmake targets available.
install (EXPORT plib-targets
	FILE plib-targets.cmake
	DESTINATION ${INSTALL_CMAKE_DIR}
	COMPONENT Core
)
