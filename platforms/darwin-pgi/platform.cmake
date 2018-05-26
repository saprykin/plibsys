if (NOT PLIBSYS_SIZEOF_VOID_P EQUAL 4)
        if (CMAKE_VERSION VERSION_LESS 2.8.10)
                message (WARNING
                         "
                         Unable to detect PGI compiler version. Beware that old
                         PGI versions doesn't support shared libraries on 64-bit
                         macOS.
                         ")
        elseif (CMAKE_C_COMPILER_VERSION VERSION_LESS 18.4)
                message (FATAL_ERROR
                         "
                         PGI compiler before verison 18.4 doesn't support 64-bit
                         shared libraries on macOS. Use older 32-bit version of
                         the compiler instead.
                         ")
        endif()
endif()

set (PLIBSYS_THREAD_MODEL posix)
set (PLIBSYS_IPC_MODEL posix)
set (PLIBSYS_TIME_PROFILER_MODEL mach)
set (PLIBSYS_DIR_MODEL posix)
set (PLIBSYS_LIBRARYLOADER_MODEL posix)

set (PLIBSYS_PLATFORM_DEFINES
        -D_REENTRANT
)
