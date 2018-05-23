# The MIT License
#
# Copyright (C) 2018 Alexander Saprykin <saprykin.spb@gmail.com>
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

project (tests CXX)
set (OUTPUT_DIR ${CMAKE_BINARY_DIR})

include (${PROJECT_SOURCE_DIR}/../cmake/PlatformDetect.cmake)
plibsys_detect_target_os (PLIBSYS_TESTS_TARGET_OS)

list (APPEND PLIBSYS_TEST_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/../src ${CMAKE_BINARY_DIR})

if (MSVC)
        list (APPEND PLIBSYS_TEST_COMPILE_DEFS -D_CRT_SECURE_NO_WARNINGS)
endif()

macro (plibsys_add_test_executable TEST_NAME SRC_FILE)
        add_executable (${TEST_NAME} ${SRC_FILE})
        target_link_libraries (${TEST_NAME} plibsys)
        set_target_properties (${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

        # QNX requires libm for sqrt() and friends
        if (PLIBSYS_TESTS_TARGET_OS STREQUAL qnx)
                target_link_libraries (${TEST_NAME} m)
        endif()

        # Add include directories
        if (COMMAND target_include_directories)
                target_include_directories (${TEST_NAME} PUBLIC ${PLIBSYS_TEST_INCLUDE_DIRS})
        else()
                include_directories (${PLIBSYS_TEST_INCLUDE_DIRS})
        endif()

        # Add compile definitions
        if (PLIBSYS_TEST_COMPILE_DEFS)
                if (COMMAND target_compile_definitions)
                        target_compile_definitions (${TEST_NAME} PRIVATE ${PLIBSYS_TEST_COMPILE_DEFS})
                else()
                        add_definitions (${PLIBSYS_TEST_COMPILE_DEFS})
                endif()
        endif()

        if (${TEST_NAME} STREQUAL "plibraryloader_test")
                add_test (NAME ${TEST_NAME} COMMAND ${TEST_NAME} -- "$<TARGET_FILE:plibsys>")
        else()
                add_test (NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        endif()
endmacro()

plibsys_add_test_executable (patomic_test patomic_test.cpp)
plibsys_add_test_executable (pcondvariable_test pcondvariable_test.cpp)
plibsys_add_test_executable (pcryptohash_test pcryptohash_test.cpp)
plibsys_add_test_executable (perror_test perror_test.cpp)
plibsys_add_test_executable (pdir_test pdir_test.cpp)
plibsys_add_test_executable (pfile_test pfile_test.cpp)
plibsys_add_test_executable (phashtable_test phashtable_test.cpp)
plibsys_add_test_executable (pinifile_test pinifile_test.cpp)
plibsys_add_test_executable (plibraryloader_test plibraryloader_test.cpp)
plibsys_add_test_executable (plist_test plist_test.cpp)
plibsys_add_test_executable (pmacros_test pmacros_test.cpp)
plibsys_add_test_executable (pmain_test pmain_test.cpp)
plibsys_add_test_executable (pmem_test pmem_test.cpp)
plibsys_add_test_executable (pmutex_test pmutex_test.cpp)
plibsys_add_test_executable (pprocess_test pprocess_test.cpp)
plibsys_add_test_executable (prwlock_test prwlock_test.cpp)
plibsys_add_test_executable (psemaphore_test psemaphore_test.cpp)
plibsys_add_test_executable (pshmbuffer_test pshmbuffer_test.cpp)
plibsys_add_test_executable (pshm_test pshm_test.cpp)
plibsys_add_test_executable (psocket_test psocket_test.cpp)
plibsys_add_test_executable (psocketaddress_test psocketaddress_test.cpp)
plibsys_add_test_executable (pspinlock_test pspinlock_test.cpp)
plibsys_add_test_executable (pstdarg_test pstdarg_test.cpp)
plibsys_add_test_executable (pstring_test pstring_test.cpp)
plibsys_add_test_executable (ptimeprofiler_test ptimeprofiler_test.cpp)
plibsys_add_test_executable (ptree_test ptree_test.cpp)
plibsys_add_test_executable (ptypes_test ptypes_test.cpp)
plibsys_add_test_executable (puthread_test puthread_test.cpp)
