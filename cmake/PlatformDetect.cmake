function (plib_detect_c_compiler result)
        # Get target system OS
        string (TOLOWER ${CMAKE_SYSTEM_NAME} PLIB_TARGET_OS)

        # Detect C compiler by it's ID
        if (CMAKE_C_COMPILER_ID STREQUAL GNU)
                set (PLIB_C_COMPILER gcc)
        elseif (CMAKE_C_COMPILER_ID STREQUAL MSVC)
                set (PLIB_C_COMPILER msvc)
        else()
                string (TOLOWER ${CMAKE_C_COMPILER_ID} PLIB_C_COMPILER)
        endif()

        # Fix gcc -> qcc naming on QNX 6
        if ((PLIB_TARGET_OS STREQUAL qnx) AND (PLIB_C_COMPILER STREQUAL gcc))
                set (PLIB_C_COMPILER qcc)
        endif()

        # Rename intel -> icc
        if (PLIB_C_COMPILER STREQUAL intel)
                set (PLIB_C_COMPILER icc)
        endif()

        # Rename openwatcom -> watcom
        if (PLIB_C_COMPILER STREQUAL openwatcom)
                set (PLIB_C_COMPILER watcom)
        endif()

        # Assign result
        set (${result} ${PLIB_C_COMPILER} PARENT_SCOPE)
endfunction (plib_detect_c_compiler)

function (plib_detect_os_bits result)
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
                set (PLIB_ARCH_BITS 64)
        else()
                set (PLIB_ARCH_BITS 32)
        endif()

        set (${result} ${PLIB_ARCH_BITS} PARENT_SCOPE)
endfunction (plib_detect_os_bits)

function (plib_detect_cpu_arch result)
        if (CMAKE_SYSTEM_PROCESSOR MATCHES "(i[1-9]86)|(x86_64)")
                if (CMAKE_CROSSCOMPILING)
                        if (CMAKE_SYSTEM_PROCESSOR MATCHES "i[1-9]86")
                                set (PLIB_PROCESSOR_ARCH "x86")
                        else()
                                set (PLIB_PROCESSOR_ARCH "x64")
                        endif()
                else()
                        plib_detect_os_bits (PLIB_OS_BITS)
                        if (PLIB_OS_BITS STREQUAL "32")
                                set (PLIB_PROCESSOR_ARCH "x86")
                        else()
                                set (PLIB_PROCESSOR_ARCH "x64")
                        endif()
                endif()
        else()
                set (PLIB_PROCESSOR_ARCH ${CMAKE_SYSTEM_PROCESSOR})
        endif()

        set (${result} ${PLIB_PROCESSOR_ARCH} PARENT_SCOPE)
endfunction (plib_detect_cpu_arch)

function (plib_detect_target_os result)
        string (TOLOWER ${CMAKE_SYSTEM_NAME} PLIB_TARGET_OS)

        if (PLIB_TARGET_OS MATCHES "(mingw.*)|(msys)")
                set (PLIB_TARGET_OS windows)
        endif()

        if (PLIB_TARGET_OS STREQUAL hp-ux)
                set (PLIB_TARGET_OS hpux)
        elseif (PLIB_TARGET_OS STREQUAL sco_sv)
                set (PLIB_TARGET_OS scosv)
        endif()

        set (${result} ${PLIB_TARGET_OS} PARENT_SCOPE)
endfunction (plib_detect_target_os)

function (plib_detect_target_platform result)
        plib_detect_target_os (PLIB_TARGET_OS)
        plib_detect_os_bits (PLIB_OS_BITS)
        plib_detect_c_compiler (PLIB_C_COMPILER)

        if (PLIB_TARGET_OS STREQUAL windows)
                set (PLIB_TARGET_PLATFORM win${PLIB_OS_BITS})
        else()
                set (PLIB_TARGET_PLATFORM ${PLIB_TARGET_OS})
        endif()

        set (PLIB_TARGET_PLATFORM ${PLIB_TARGET_PLATFORM}-${PLIB_C_COMPILER})
        set (${result} ${PLIB_TARGET_PLATFORM} PARENT_SCOPE)
endfunction (plib_detect_target_platform)
