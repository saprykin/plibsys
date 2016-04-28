# Look for PLibSYS
find_library (PLibSYS_LIBRARIES plibsys PATHS ${CMAKE_INSTALL_PREFIX}/../plibsys/lib)

if (PLibSYS_LIBRARIES)
        get_filename_component (PLibSYS_LIBRARY_DIRS ${PLibSYS_LIBRARIES} PATH)
        get_filename_component (PLIBSYS_ROOT_DIR ${PLibSYS_LIBRARY_DIRS} PATH)
        set (PLibSYS_INCLUDE_DIRS ${PLIBSYS_ROOT_DIR}/include)
        unset (PLIBSYS_ROOT_DIR)
        if (NOT EXISTS ${PLibSYS_INCLUDE_DIRS}/PLibSYS/plibsys.h)
                unset (PLibSYS_INCLUDE_DIRS)
                unset (PLibSYS_LIBRARIES)
                unset (PLibSYS_LIBRARY_DIRS)
        endif()
endif()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (PLibSYS DEFAULT_MSG PLibSYS_LIBRARY_DIRS PLibSYS_LIBRARIES PLibSYS_INCLUDE_DIRS)
mark_as_advanced (PLibSYS_LIBRARY_DIRS PLibSYS_LIBRARIES PLibSYS_INCLUDE_DIRS)
