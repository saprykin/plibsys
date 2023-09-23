set (PLIBSYS_THREAD_MODEL posix)
set (PLIBSYS_IPC_MODEL posix)
set (PLIBSYS_TIME_PROFILER_MODEL posix)
set (PLIBSYS_DIR_MODEL posix)
set (PLIBSYS_LIBRARYLOADER_MODEL posix)

set (PLIBSYS_PLATFORM_LINK_LIBRARIES rt dl)
set (PLIBSYS_PLATFORM_CFLAGS "-h threadsafe -h nomessage=186 -O1")

set (PLIBSYS_PLATFORM_DEFINES
        -D_GNU_SOURCE
)

message ("
         Cray compiler has an optimization bug on SHA-1 algorithm
         when using optimization levels higher than O1. Thus
         O1 is enabled by default.
         ")
