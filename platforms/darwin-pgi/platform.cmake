if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        message (FATAL_ERROR
                 "
                 PGI compiler doesn't support 64-bit shared libraries on 
                 macOS. Use 32-bit version of the compiler instead.
                 ")
endif()

set (PLIBSYS_THREAD_MODEL posix)
set (PLIBSYS_IPC_MODEL posix)
set (PLIBSYS_TIME_PROFILER_MODEL mach)
set (PLIBSYS_DIR_MODEL posix)
set (PLIBSYS_LIBRARYLOADER_MODEL posix)

set (PLIBSYS_PLATFORM_DEFINES
        -D_REENTRANT
)
