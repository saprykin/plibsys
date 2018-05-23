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

function (plibsys_detect_c_compiler result)
        # Get target system OS
        string (TOLOWER ${CMAKE_SYSTEM_NAME} PLIBSYS_TARGET_OS)

        # Detect C compiler by it's ID
        if (CMAKE_C_COMPILER_ID STREQUAL GNU)
                set (PLIBSYS_C_COMPILER gcc)
        elseif (CMAKE_C_COMPILER_ID STREQUAL MSVC)
                set (PLIBSYS_C_COMPILER msvc)
        else()
                string (TOLOWER ${CMAKE_C_COMPILER_ID} PLIBSYS_C_COMPILER)
        endif()

        # Fix gcc -> qcc naming on QNX 6
        if ((PLIBSYS_TARGET_OS STREQUAL qnx) AND (PLIBSYS_C_COMPILER STREQUAL gcc))
                set (PLIBSYS_C_COMPILER qcc)
        endif()

        # Rename intel -> icc
        if (PLIBSYS_C_COMPILER STREQUAL intel)
                set (PLIBSYS_C_COMPILER icc)
        endif()

        # Rename openwatcom -> watcom
        if (PLIBSYS_C_COMPILER STREQUAL openwatcom)
                set (PLIBSYS_C_COMPILER watcom)
        endif()

        # Rename xl -> xlc
        if (PLIBSYS_C_COMPILER STREQUAL xl)
                set (PLIBSYS_C_COMPILER xlc)
        endif()

        # Assign result
        set (${result} ${PLIBSYS_C_COMPILER} PARENT_SCOPE)
endfunction (plibsys_detect_c_compiler)

function (plibsys_detect_os_bits result)
        if (PLIBSYS_SIZEOF_VOID_P EQUAL 8)
                set (PLIBSYS_ARCH_BITS 64)
        elseif (PLIBSYS_SIZEOF_VOID_P EQUAL 4)
                set (PLIBSYS_ARCH_BITS 32)
        elseif(PLIBSYS_SIZEOF_VOID_P EQUAL 0)
                set (PLIBSYS_ARCH_BITS "universal")
        else()
                set (PLIBSYS_ARCH_BITS "unknown")
        endif()

        set (${result} ${PLIBSYS_ARCH_BITS} PARENT_SCOPE)
endfunction (plibsys_detect_os_bits)

function (plibsys_detect_cpu_arch result)
        if (CMAKE_SYSTEM_PROCESSOR MATCHES "(i[1-9]86)|(x86_64)")
                if (CMAKE_CROSSCOMPILING)
                        if (CMAKE_SYSTEM_PROCESSOR MATCHES "i[1-9]86")
                                set (PLIBSYS_PROCESSOR_ARCH "x86")
                        else()
                                set (PLIBSYS_PROCESSOR_ARCH "x64")
                        endif()
                else()
                        plibsys_detect_os_bits (PLIBSYS_OS_BITS)
                        if (PLIBSYS_OS_BITS STREQUAL "32")
                                set (PLIBSYS_PROCESSOR_ARCH "x86")
                        elseif (PLIBSYS_OS_BITS STREQUAL "64")
                                set (PLIBSYS_PROCESSOR_ARCH "x64")
                        else()
                                set (PLIBSYS_PROCESSOR_ARCH "x${PLIBSYS_OS_BITS}")
                        endif()
                endif()
        else()
                set (PLIBSYS_PROCESSOR_ARCH ${CMAKE_SYSTEM_PROCESSOR})
        endif()

        set (${result} ${PLIBSYS_PROCESSOR_ARCH} PARENT_SCOPE)
endfunction (plibsys_detect_cpu_arch)

function (plibsys_detect_target_os result)
        string (TOLOWER ${CMAKE_SYSTEM_NAME} PLIBSYS_TARGET_OS)

        # Rename mingw -> windows
        if (PLIBSYS_TARGET_OS MATCHES "(mingw.*)")
                set (PLIBSYS_TARGET_OS windows)
        endif()

        # Rename hp-ux -> hpux
        if (PLIBSYS_TARGET_OS STREQUAL hp-ux)
                set (PLIBSYS_TARGET_OS hpux)
        endif()

        # Rename sco_sv -> scosv
        if (PLIBSYS_TARGET_OS STREQUAL sco_sv)
                set (PLIBSYS_TARGET_OS scosv)
        endif()

        # Rename osf1 -> tru64
        if (PLIBSYS_TARGET_OS STREQUAL osf1)
                set (PLIBSYS_TARGET_OS tru64)
        endif()

        # Rename craylinuxenvironment -> linux
        if (PLIBSYS_TARGET_OS STREQUAL craylinuxenvironment)
                set (PLIBSYS_TARGET_OS linux)
        endif()

        set (${result} ${PLIBSYS_TARGET_OS} PARENT_SCOPE)
endfunction (plibsys_detect_target_os)

function (plibsys_detect_target_platform result)
        plibsys_detect_target_os (PLIBSYS_TARGET_OS)
        plibsys_detect_os_bits (PLIBSYS_OS_BITS)
        plibsys_detect_c_compiler (PLIBSYS_C_COMPILER)

        if (PLIBSYS_TARGET_OS STREQUAL windows)
                set (PLIBSYS_TARGET_PLATFORM win${PLIBSYS_OS_BITS})
        else()
                set (PLIBSYS_TARGET_PLATFORM ${PLIBSYS_TARGET_OS})
        endif()

        set (PLIBSYS_TARGET_PLATFORM ${PLIBSYS_TARGET_PLATFORM}-${PLIBSYS_C_COMPILER})
        set (${result} ${PLIBSYS_TARGET_PLATFORM} PARENT_SCOPE)
endfunction (plibsys_detect_target_platform)
