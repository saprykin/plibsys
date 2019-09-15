# The MIT License
#
# Copyright (C) 2018-2019 Alexander Saprykin <saprykin.spb@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# 'Software'), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


set (CMAKE_LEGACY_CYGWIN_WIN32 0)

cmake_minimum_required (VERSION 2.8.0)
project (plibsys C)

set (PLIBSYS_VERSION_MAJOR 0)
set (PLIBSYS_VERSION_MINOR 0)
set (PLIBSYS_VERSION_PATCH 4)
set (PLIBSYS_VERSION_NUM 0x000004)
set (PLIBSYS_VERSION ${PLIBSYS_VERSION_MAJOR}.${PLIBSYS_VERSION_MINOR}.${PLIBSYS_VERSION_PATCH})
set (PLIBSYS_SOVERSION ${PLIBSYS_VERSION_MAJOR})
set (VERSION ${PLIBSYS_VERSION})

set (top_srcdir ${PROJECT_SOURCE_DIR})

option (PLIBSYS_TESTS "Build unit tests" ON)
option (PLIBSYS_BUILD_STATIC "Also build static version of the library" ON)
option (PLIBSYS_COVERAGE "Enable gcov coverage (GCC and Clang)" OFF)
option (PLIBSYS_VISIBILITY "Use explicit symbols visibility if possible" ON)
option (PLIBSYS_BUILD_DOC "Enable building HTML documentation" ON)

if (NOT CMAKE_BUILD_TYPE)
        set (CMAKE_BUILD_TYPE "Debug")
endif()

subdirs (src)

if (PLIBSYS_TESTS)
        enable_testing ()
        subdirs (tests)
endif()

if (PLIBSYS_BUILD_DOC)
        find_package (Doxygen)

        if (DOXYGEN_FOUND)
                configure_file (${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile)

                add_custom_target (doc ALL
                        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
                        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                        COMMENT "Generating API documentation with Doxygen"
                        VERBATIM
                )

                install (DIRECTORY ${PROJECT_BINARY_DIR}/doc/html DESTINATION share/doc)
        endif()
endif()

set (CPACK_PACKAGE_NAME ${PROJECT_NAME}-installer)
set (CPACK_PACKAGE_VENDOR "Alexander Saprykin")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} - System routines library")
set (CPACK_PACKAGE_VERSION_MAJOR ${PLIBSYS_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${PLIBSYS_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${PLIBSYS_VERSION_PATCH})
set (CPACK_PACKAGE_VERSION ${PLIBSYS_VERSION})
set (CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME}-${CPACK_PACKAGE_VERSION})
set (CPACK_COMPONENTS_ALL Core)
set (CPACK_COMPONENT_CORE_DISPLAY_NAME "Core components")
set (CPACK_COMPONENT_CORE_DESCRIPTION "Core library with headers")
set (CPACK_COMPONENT_CORE_REQUIRED TRUE)

if (WIN32 AND NOT UNIX)
        set (CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})
elseif (UNIX)
        set (CPACK_GENERATOR STGZ)
endif()

include (CPack)
