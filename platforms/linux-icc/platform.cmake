set (PLIB_THREAD_MODEL posix)
set (PLIB_IPC_MODEL posix)
set (PLIB_TIME_PROFILER_MODEL posix)
set (PLIB_DIR_MODEL posix)

set (PLIB_PLATFORM_LINK_LIBRARIES -pthread rt dl imf svml irng intlc)

set (PLIB_PLATFORM_DEFINES
        -D_REENTRANT
)
