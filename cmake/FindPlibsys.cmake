# Look for plibsys
find_library (plibsys_LIBRARIES plibsys PATHS ${CMAKE_INSTALL_PREFIX}/../plibsys/lib)

if (plibsys_LIBRARIES)
        get_filename_component (plibsys_LIBRARY_DIRS ${plibsys_LIBRARIES} PATH)
        get_filename_component (PLIBSYS_ROOT_DIR ${plibsys_LIBRARY_DIRS} PATH)
        set (plibsys_INCLUDE_DIRS ${PLIBSYS_ROOT_DIR}/include)
        unset (PLIBSYS_ROOT_DIR)
        if (NOT EXISTS ${plibsys_INCLUDE_DIRS}/plibsys/plibsys.h)
                unset (plibsys_INCLUDE_DIRS)
                unset (plibsys_LIBRARIES)
                unset (plibsys_LIBRARY_DIRS)
        endif()
endif()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (plibsys DEFAULT_MSG plibsys_LIBRARY_DIRS plibsys_LIBRARIES plibsys_INCLUDE_DIRS)
mark_as_advanced (plibsys_LIBRARY_DIRS plibsys_LIBRARIES plibsys_INCLUDE_DIRS)
