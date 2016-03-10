set (PLIB_THREAD_MODEL posix)
set (PLIB_IPC_MODEL sysv)
set (PLIB_TIME_PROFILER_MODEL posix)
set (PLIB_DIR_MODEL posix)

set (PLIB_PLATFORM_DEFINES
        -D_REENTRANT
        -D_SIMPLE_R
)

if (CMAKE_SYSTEM_VERSION VERSION_LESS "5.0")
        set (PLIB_PLATFORM_LINK_LIBRARIES socket nsl gthreads malloc)

        message ("
                 You need a working port of FSU Pthreads in order to
                 compile with multi-threading support. Please refer to
                 http://moss.csc.ncsu.edu/~mueller/pthreads/ for more
                 details. Make sure that it is compiled with thread-safe
                 memory allocation (usually -DMALLOC macro definition).
                 ")
else()
        set (PLIB_PLATFORM_LINK_LIBRARIES socket nsl -pthread)
        set (PLIB_PLATFORM_CFLAGS -pthread)

        message ("
                 SCO OpenServer 6 was not actually tested with GCC
                 compiler. This build may or may not work properly.
                 Consider running tests before usage.
                 ")
endif()
