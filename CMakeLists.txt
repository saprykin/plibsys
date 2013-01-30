cmake_minimum_required(VERSION 2.8)
project(plib C)
set(VERSION "0.0.1")

if(!MSVC)
	message(FATAL_ERROR "CMake build system is only available for Windows (MSVC) targets")
endif()

set(top_srcdir ${PROJECT_SOURCE_DIR})

configure_file (
	"${PROJECT_SOURCE_DIR}/Doxyfile.in"
	"${PROJECT_SOURCE_DIR}/Doxyfile"
)

subdirs(src)