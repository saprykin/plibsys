include (${PROJECT_SOURCE_DIR}/platforms/common/SCOSV.cmake)

set (PLIBSYS_THREAD_MODEL posix)
set (PLIBSYS_IPC_MODEL sysv)
set (PLIBSYS_TIME_PROFILER_MODEL posix)
set (PLIBSYS_DIR_MODEL posix)
set (PLIBSYS_LIBRARYLOADER_MODEL posix)

set (PLIBSYS_PLATFORM_DEFINES
        -D_REENTRANT
        -D_SIMPLE_R
)

if (CMAKE_SYSTEM_VERSION VERSION_LESS "5.0")
        set (PLIBSYS_PLATFORM_LINK_LIBRARIES socket nsl gthreads malloc)

        plibsys_scosv_print_threading_message ()
else()
        set (PLIBSYS_PLATFORM_LINK_LIBRARIES socket nsl -pthread)
        set (PLIBSYS_PLATFORM_CFLAGS -pthread)

        message ("
                 SCO OpenServer 6 was not actually tested with GCC
                 compiler. This build may or may not work properly.
                 Consider running tests before usage.
                 ")
endif()
