set (CMAKE_LEGACY_CYGWIN_WIN32 0)

cmake_minimum_required (VERSION 2.8.0)
project (plibsys C)

set (PLIBSYS_VERSION_MAJOR 0)
set (PLIBSYS_VERSION_MINOR 0)
set (PLIBSYS_VERSION_PATCH 1)
set (PLIBSYS_VERSION_NUM 0x000001)
set (PLIBSYS_VERSION ${PLIBSYS_VERSION_MAJOR}.${PLIBSYS_VERSION_MINOR}.${PLIBSYS_VERSION_PATCH})
set (VERSION ${PLIBSYS_VERSION})

set (top_srcdir ${PROJECT_SOURCE_DIR})

configure_file (
        "${PROJECT_SOURCE_DIR}/Doxyfile.in"
        "${PROJECT_BINARY_DIR}/Doxyfile"
)

option (PLIBSYS_TESTS "Build unit tests" ON)
option (PLIBSYS_TESTS_STATIC "Use Boost Unit Test Framework without compilation" OFF)
option (PLIBSYS_BUILD_STATIC "Also build static version of the library" ON)
option (PLIBSYS_COVERAGE "Enable gcov coverage (GCC and Clang)" OFF)

if (NOT CMAKE_BUILD_TYPE)
        set (CMAKE_BUILD_TYPE "Debug")
endif()

subdirs (src)

if (PLIBSYS_TESTS)
        message (STATUS "Checking whether to enable unit tests")

        enable_testing ()

        if (BOOST_ROOT)
                if (NOT IS_ABSOLUTE "${BOOST_ROOT}")
                        get_filename_component (BOOST_ROOT_ABSOLUTE "${CMAKE_BINARY_DIR}/${BOOST_ROOT}" ABSOLUTE)
                        set (BOOST_ROOT ${BOOST_ROOT_ABSOLUTE})
                endif()
        endif()

        find_package (Boost COMPONENTS unit_test_framework)

        if (Boost_FOUND)
                if (PLIBSYS_TESTS_STATIC)
                        message (STATUS "Checking whether to enable unit tests (static) - yes")
                else()
                        message (STATUS "Checking whether to enable unit tests - yes")
                endif() 

                subdirs (tests)
        else()
                if (PLIBSYS_TESTS_STATIC)
                        if (BOOST_ROOT)
                                message (STATUS "Checking whether to enable unit tests (static) - yes")
                        else()
                                message (FATAL_ERROR "You must specify BOOST_ROOT variable when building static tests without proper Boost installation")
                        endif()

                        subdirs (tests)
                else()
                        message (STATUS "Checking whether to enable unit tests - no")
                endif()
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
