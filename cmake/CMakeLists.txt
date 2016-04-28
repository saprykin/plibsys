# Config file support for find_package(PLibSYS). This needs to
# deal with two environments: (1) finding the build tree and (2)
# finding the install tree. plibsys-config.cmake detects which
# situation it is handing by looking at @PLIBSYS_ROOT_DIR@. If
# this is an absolute path, it's in the build tree; otherwise,
# it's in the install tree (note that the whole install tree
# can be relocated).

include (PlatformDetect.cmake)

plibsys_detect_target_platform (PLIBSYS_TARGET_PLATFORM)

if (PLIBSYS_BUILD_STATIC)
        set (PLIBSYS_INSTALL_TARGETS plibsys plibsysstatic)
else()
        set (PLIBSYS_INSTALL_TARGETS plibsys)
endif()

# plibsys-config.cmake for the build tree.
set(PLIBSYS_ROOT_DIR ${PROJECT_BINARY_DIR})
set(PLibSYS_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/platforms/${PLIBSYS_TARGET_PLATFORM})
configure_file(plibsys-config.cmake.in ${PROJECT_BINARY_DIR}/plibsys-config.cmake @ONLY)
configure_file(plibsys-config-version.cmake.in ${PROJECT_BINARY_DIR}/plibsys-config-version.cmake @ONLY)
export (TARGETS ${PLIBSYS_INSTALL_TARGETS} FILE ${PROJECT_BINARY_DIR}/plibsys-targets.cmake)

# plibsys-config.cmake for the install tree. It's installed in
# ${INSTALL_CMAKE_DIR} and @PLIBSYS_ROOT_DIR@ is the relative
# path to the root from there (note that the whole install tree can
# be relocated).
set (INSTALL_CMAKE_DIR cmake)
set (PLIBSYS_ROOT_DIR ..)

# @PLibSYS_INCLUDE_DIRS@ is not used in the install tree; reset
# it to prevent the source and build paths appearing in the installed
# config files.
set (PLibSYS_INCLUDE_DIRS IGNORE)
configure_file (plibsys-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/plibsys-config.cmake @ONLY)
configure_file (plibsys-config-version.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/plibsys-config-version.cmake @ONLY)
install (FILES
        ${CMAKE_CURRENT_BINARY_DIR}/plibsys-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/plibsys-config-version.cmake
        DESTINATION ${INSTALL_CMAKE_DIR}
        COMPONENT Core
)

# Make information about the cmake targets available.
install (EXPORT plibsys-targets
        FILE plibsys-targets.cmake
        DESTINATION ${INSTALL_CMAKE_DIR}
        COMPONENT Core
)
