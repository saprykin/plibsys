set (PLIB_THREAD_MODEL posix)
set (PLIB_IPC_MODEL sysv)
set (PLIB_TIME_PROFILER_MODEL posix)
set (PLIB_DIR_MODEL posix)

set (PLIB_PLATFORM_LINK_LIBRARIES socket nsl gthreads)

set (PLIB_PLATFORM_DEFINES
        -D_REENTRANT
        -D_SIMPLE_R
)

message ("
         You need a working port of FSU Pthreads in order to
         compile with multi-threading support. Please refer to
         http://moss.csc.ncsu.edu/~mueller/pthreads/ for more
         details. Make sure that it is compiled with thread-safe
         memory allocation (usually -DMALLOC macro definition).
         ")
