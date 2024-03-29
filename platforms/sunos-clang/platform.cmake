set (PLIBSYS_THREAD_MODEL solaris)
set (PLIBSYS_IPC_MODEL sysv)
set (PLIBSYS_TIME_PROFILER_MODEL solaris)
set (PLIBSYS_DIR_MODEL posix)
set (PLIBSYS_LIBRARYLOADER_MODEL posix)

set (PLIBSYS_PLATFORM_LINK_LIBRARIES socket nsl rt thread)

set (PLIBSYS_PLATFORM_DEFINES
        -D_REENTRANT
        -D_POSIX_PTHREAD_SEMANTICS
)
