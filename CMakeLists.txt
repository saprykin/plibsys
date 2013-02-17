cmake_minimum_required(VERSION 2.8)
project(PLib C)
set(PLIB_VERSION_MAJOR 0)
set(PLIB_VERSION_MINOR 0)
set(PLIB_VERSION_PATCH 1)
set(PLIB_VERSION ${PLIB_VERSION_MAJOR}.${PLIB_VERSION_MINOR}.${PLIB_VERSION_PATCH})
set(VERSION ${PLIB_VERSION})

if(!MSVC)
	message(FATAL_ERROR "CMake build system is only available for Windows (MSVC) targets")
endif()

set(top_srcdir ${PROJECT_SOURCE_DIR})

configure_file (
	"${PROJECT_SOURCE_DIR}/Doxyfile.in"
	"${PROJECT_SOURCE_DIR}/Doxyfile"
)

subdirs(src)
subdirs(cmake)