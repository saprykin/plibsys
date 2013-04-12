# Look for PLib
find_library (PLib_LIBRARIES plib PATHS ${CMAKE_INSTALL_PREFIX}/../plib/lib)

if (PLib_LIBRARIES)
	get_filename_component (PLib_LIBRARY_DIRS ${PLib_LIBRARIES} PATH)
	get_filename_component (PLIB_ROOT_DIR ${PLib_LIBRARY_DIRS} PATH)
	set (PLib_INCLUDE_DIRS ${PLIB_ROOT_DIR}/include)
	unset (PLIB_ROOT_DIR)
	if (NOT EXISTS ${PLib_INCLUDE_DIRS}/PLib/plib.h)
		unset (PLib_INCLUDE_DIRS)
		unset (PLib_LIBRARIES)
		unset (PLib_LIBRARY_DIRS)
	endif()
endif()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (PLib DEFAULT_MSG PLib_LIBRARY_DIRS PLib_LIBRARIES PLib_INCLUDE_DIRS)
mark_as_advanced (PLib_LIBRARY_DIRS PLib_LIBRARIES PLib_INCLUDE_DIRS)
