set (PLIB_THREAD_MODEL posix)
set (PLIB_IPC_MODEL posix)
set (PLIB_TIME_PROFILER_MODEL mach)
set (PLIB_DIR_MODEL posix)
set (PLIB_PLATFORM_CONFIG_PATH "../common/posix/pplatform.h")

set (PLIB_PLATFORM_DEFINES
        -D_REENTRANT
        -D_POSIX4_DRAFT_SOURCE
        -D_POSIX4_DRAFT10_SOURCE
        -DU_OSF_SOURCE
)
