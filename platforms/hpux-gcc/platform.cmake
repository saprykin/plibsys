include (${PROJECT_SOURCE_DIR}/platforms/common/HPUX.cmake)

set (PLIBSYS_THREAD_MODEL posix)
set (PLIBSYS_IPC_MODEL sysv)
set (PLIBSYS_TIME_PROFILER_MODEL solaris)
set (PLIBSYS_DIR_MODEL posix)

plibsys_hpux_detect_libraryloader_model (PLIBSYS_LIBRARYLOADER_MODEL)

set (PLIBSYS_PLATFORM_LINK_LIBRARIES xnet rt -pthread)

set (PLIBSYS_PLATFORM_DEFINES
        -D_REENTRANT
        -D_THREAD_SAFE
        -D_XOPEN_SOURCE_EXTENDED=1
)
