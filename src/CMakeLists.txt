# The MIT License
#
# Copyright (C) 2018 Alexander Saprykin <saprykin.spb@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# 'Software'), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

include (CheckCSourceCompiles)
include (CheckTypeSize)
include (CheckIncludeFile)
include (TestBigEndian)

# First of all, detect size of the pointer for the target machine.
# We can't rely on CMAKE_SIZEOF_VOID_P when building universal
# binaries.

check_type_size ("void *" PLIBSYS_SIZEOF_VOID_P)
check_type_size ("size_t" PLIBSYS_SIZEOF_SIZE_T)
check_type_size ("long" PLIBSYS_SIZEOF_LONG)

# Without generated code multi-arch builds are not supported anyway,
# so the best we can do is to fallback to detected size values

if (CMAKE_VERSION VERSION_LESS 2.8.1)
        set (PLIBSYS_SIZEOF_VOID_P_CODE "#define PLIBSYS_SIZEOF_VOID_P ${PLIBSYS_SIZEOF_VOID_P}")
        set (PLIBSYS_SIZEOF_SIZE_T_CODE "#define PLIBSYS_SIZEOF_SIZE_T ${PLIBSYS_SIZEOF_SIZE_T}")
        set (PLIBSYS_SIZEOF_LONG_CODE "#define PLIBSYS_SIZEOF_LONG ${PLIBSYS_SIZEOF_LONG}")
endif()

include (${PROJECT_SOURCE_DIR}/cmake/PlatformDetect.cmake)
include (${PROJECT_SOURCE_DIR}/cmake/VisibilityDetect.cmake)
include (${PROJECT_SOURCE_DIR}/cmake/StdargDetect.cmake)
set (OUTPUT_DIR ${CMAKE_BINARY_DIR})

# Try to detect target platform
plibsys_detect_target_platform (PLIBSYS_TARGET_PLATFORM)
plibsys_detect_c_compiler (PLIBSYS_C_COMPILER)
plibsys_detect_target_os (PLIBSYS_TARGET_OS)
plibsys_detect_os_bits (PLIBSYS_OS_BITS)

if (PLIBSYS_OS_BITS STREQUAL "unknown")
    message (FATAL_ERROR "Failed to detect bitness of the target system")
endif()

if ((PLIBSYS_TARGET_OS STREQUAL windows) AND NOT (PLIBSYS_TARGET_OS STREQUAL cygwin)
                                         AND NOT (PLIBSYS_TARGET_OS STREQUAL msys))
        set (PLIBSYS_NATIVE_WINDOWS true)
endif()

if (PLIBSYS_COVERAGE)
        if (PLIBSYS_C_COMPILER MATCHES "gcc|clang")
                set (CMAKE_BUILD_TYPE "Debug")
                list (APPEND CMAKE_C_FLAGS "--coverage")
        endif()
endif()

# CMP0042, see http://www.cmake.org/Wiki/CMake_RPATH_handling
if (PLIBSYS_TARGET_OS STREQUAL darwin)
        if (POLICY CMP0068)
                cmake_policy (SET CMP0068 NEW)
        endif()

        set (CMAKE_MACOSX_RPATH TRUE)
        set (CMAKE_SKIP_BUILD_RPATH FALSE)

        # Fix runtime paths on macOS 10.5 and less
        if (CMAKE_SYSTEM_VERSION VERSION_LESS "10.0.0")
                set (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
        else()
                set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
        endif()

        set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
        set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

        list (FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)

        if ("${isSystemDir}" STREQUAL "-1")
                set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
        endif ("${isSystemDir}" STREQUAL "-1")
endif()

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/platforms/${PLIBSYS_TARGET_PLATFORM}/")
        message (FATAL_ERROR "plibsys doesn't support unknown platform ${PLIBSYS_TARGET_PLATFORM}")
endif()

include (${PROJECT_SOURCE_DIR}/platforms/${PLIBSYS_TARGET_PLATFORM}/platform.cmake)

set (PLIBSYS_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/src
        ${CMAKE_BINARY_DIR}
)

set (PLIBSYS_PUBLIC_HDRS
        patomic.h
        ptypes.h
        pmacros.h
        pmacroscompiler.h
        pmacroscpu.h
        pmacrosos.h
        pcondvariable.h
        pcryptohash.h
        perror.h
        perrortypes.h
        pdir.h
        pfile.h
        phashtable.h
        pinifile.h
        plibsys.h
        plibraryloader.h
        plist.h
        pmain.h
        pmem.h
        pmutex.h
        pprocess.h
        prwlock.h
        psemaphore.h
        pshm.h
        pshmbuffer.h
        psocket.h
        psocketaddress.h
        pspinlock.h
        pstdarg.h
        pstring.h
        ptimeprofiler.h
        ptree.h
        puthread.h
)

set (PLIBSYS_PRIVATE_HDRS
        pcryptohash-gost3411.h
        pcryptohash-md5.h
        pcryptohash-sha1.h
        pcryptohash-sha2-256.h
        pcryptohash-sha2-512.h
        pcryptohash-sha3.h
        perror-private.h
        plibsys-private.h
        psysclose-private.h
        ptimeprofiler-private.h
        ptree-avl.h
        ptree-bst.h
        ptree-rb.h
        ptree-private.h
        puthread-private.h
        ${CMAKE_BINARY_DIR}/plibsysconfig.h
)

set (PLIBSYS_SRCS
        pcryptohash.c
        pcryptohash-gost3411.c
        pcryptohash-md5.c
        pcryptohash-sha1.c
        pcryptohash-sha2-256.c
        pcryptohash-sha2-512.c
        pcryptohash-sha3.c
        pdir.c
        perror.c
        pfile.c
        phashtable.c
        pinifile.c
        plist.c
        pmain.c
        pmem.c
        pprocess.c
        pshmbuffer.c
        psocket.c
        psocketaddress.c
        pstring.c
        ptimeprofiler.c
        ptree.c
        ptree-avl.c
        ptree-bst.c
        ptree-rb.c
        puthread.c
)

if (PLIBSYS_NATIVE_WINDOWS)
        set (PLIBSYS_CLOSE_MODEL win)
elseif (PLIBSYS_TARGET_OS STREQUAL darwin)
        set (PLIBSYS_CLOSE_MODEL darwin)
else()
        set (PLIBSYS_CLOSE_MODEL unix)
endif()

if (PLIBSYS_THREAD_MODEL STREQUAL "")
        set (PLIBSYS_THREAD_MODEL none)
endif()

if (PLIBSYS_IPC_MODEL STREQUAL "")
        set (PLIBSYS_IPC_MODEL none)
endif()

if (PLIBSYS_TIME_PROFILER_MODEL STREQUAL "")
        set (PLIBSYS_TIME_PROFILER_MODEL generic)
endif()

if (PLIBSYS_DIR_MODEL STREQUAL "")
        set (PLIBSYS_DIR_MODEL none)
endif()

if (PLIBSYS_LIBRARYLOADER_MODEL STREQUAL "")
        set (PLIBSYS_LIBRARYLOADER_MODEL none)
endif()

set (PLIBSYS_PLATFORM_SRCS
        pcondvariable-${PLIBSYS_THREAD_MODEL}.c
        pmutex-${PLIBSYS_THREAD_MODEL}.c
        psemaphore-${PLIBSYS_IPC_MODEL}.c
        pshm-${PLIBSYS_IPC_MODEL}.c
        psysclose-${PLIBSYS_CLOSE_MODEL}.c
        puthread-${PLIBSYS_THREAD_MODEL}.c
        ptimeprofiler-${PLIBSYS_TIME_PROFILER_MODEL}.c
        pdir-${PLIBSYS_DIR_MODEL}.c
        plibraryloader-${PLIBSYS_LIBRARYLOADER_MODEL}.c
)

if (NOT PLIBSYS_IPC_MODEL STREQUAL none)
        list (APPEND PLIBSYS_PRIVATE_HDRS pipc-private.h)
        list (APPEND PLIBSYS_SRCS pipc.c)
endif()

# Save compiler flags
set (SAVED_CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS})
set (SAVED_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
set (SAVED_CMAKE_REQUIRED_FLAGS ${SAVED_CMAKE_REQUIRED_FLAGS})

list (APPEND CMAKE_REQUIRED_DEFINITIONS ${PLIBSYS_PLATFORM_DEFINES})
list (APPEND CMAKE_REQUIRED_LIBRARIES ${PLIBSYS_PLATFORM_LINK_LIBRARIES})
list (APPEND CMAKE_REQUIRED_FLAGS ${PLIBSYS_PLATFORM_LDFLAGS})

set (PLIBSYS_ATOMIC_LOCK_FREE no)

if (NOT PLIBSYS_ATOMIC_MODEL)
        message (STATUS "Checking for lock-free atomic intrinsics")

        if (NOT PLIBSYS_C_COMPILER STREQUAL clang)
                # GCC __atomic* intrinsics
                check_c_source_compiles (
                                         "int main () {
                                          int i, tmp_int = 0;
                                          void *ptr, *tmp_ptr = 0;
                                          __atomic_store_4 (&i, 0, __ATOMIC_SEQ_CST);
                                          __atomic_compare_exchange_n (&i, &tmp_int, 1, 0,
                                                                       __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
                                          __atomic_compare_exchange_n ((unsigned long long *) &ptr,
                                                                       (unsigned long long *) &tmp_ptr,
                                                                       (unsigned long long) 1, 0,
                                                                       __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
                                          __atomic_fetch_add (&i, 1, __ATOMIC_SEQ_CST);
                                          return 0;
                                         }"
                                         PLIBSYS_ATOMIC_IMPL_GCCATOMIC
                )

                if (PLIBSYS_ATOMIC_IMPL_GCCATOMIC)
                        set (PLIBSYS_ATOMIC_LOCK_FREE yes)
                        set (PLIBSYS_ATOMIC_MODEL "c11")
                endif()
        endif()

        # GCC __sync* intinsics
        if (NOT PLIBSYS_ATOMIC_LOCK_FREE)
                check_c_source_compiles (
                                         "int main () {
                                          int i;
                                          __sync_synchronize ();
                                          __sync_val_compare_and_swap (&i, 0, 1);
                                          __sync_fetch_and_add (&i, 1);
                                          return 0;
                                         }"
                                         PLIBSYS_ATOMIC_IMPL_GCCSYNC
                )

                if (PLIBSYS_ATOMIC_IMPL_GCCSYNC)
                        set (PLIBSYS_ATOMIC_LOCK_FREE yes)
                        set (PLIBSYS_ATOMIC_MODEL "sync")
                endif()
        endif()

        if (NOT PLIBSYS_ATOMIC_LOCK_FREE)
                if (PLIBSYS_TARGET_OS STREQUAL windows AND (PLIBSYS_C_COMPILER STREQUAL borland OR
                                                            PLIBSYS_C_COMPILER STREQUAL watcom  OR
                                                            PLIBSYS_C_COMPILER STREQUAL icc     OR
                                                            PLIBSYS_C_COMPILER STREQUAL msvc))
                        set (PLIBSYS_ATOMIC_LOCK_FREE true)
                        set (PLIBSYS_ATOMIC_MODEL "win")
                else()
                        set (PLIBSYS_ATOMIC_LOCK_FREE no)
                        set (PLIBSYS_ATOMIC_MODEL "sim")
                endif()
        endif()

        if (PLIBSYS_ATOMIC_LOCK_FREE)
                message (STATUS "Checking for lock-free atomic intrinsics - works")
        else()
                message (STATUS "Checking for lock-free atomic intrinsics - not works")
        endif()
endif()

list (APPEND PLIBSYS_SRCS
        patomic-${PLIBSYS_ATOMIC_MODEL}.c
        pspinlock-${PLIBSYS_ATOMIC_MODEL}.c
)

if (EXISTS PLIBSYS_CONFIG_FILE)
        file (REMOVE ${PLIBSYS_CONFIG_FILE})
endif()

test_big_endian (PLIBSYS_IS_BIGENDIAN)

check_include_file ("float.h" PLIBSYS_HAVE_FLOAT_H)
check_include_file ("values.h" PLIBSYS_HAVE_VALUES_H)
check_include_file ("limits.h" PLIBSYS_HAVE_LIMITS_H)

if (PLIBSYS_HAVE_FLOAT_H)
        set (PLIBSYS_NEED_FLOAT_H TRUE)
        set (PLIBSYS_FLOAT_MIN FLT_MIN)
        set (PLIBSYS_FLOAT_MAX FLT_MAX)
        set (PLIBSYS_DOUBLE_MIN DBL_MIN)
        set (PLIBSYS_DOUBLE_MAX DBL_MAX)
elseif (PLIBSYS_HAVE_VALUES_H)
        set (PLIBSYS_NEED_VALUES_H TRUE)
        set (PLIBSYS_FLOAT_MIN MINFLOAT)
        set (PLIBSYS_FLOAT_MAX MAXFLOAT)
        set (PLIBSYS_DOUBLE_MIN MINDOUBLE)
        set (PLIBSYS_DOUBLE_MAX MAXDOUBLE)
endif()

if (PLIBSYS_HAVE_LIMITS_H)
        set (PLIBSYS_NEED_LIMITS_H TRUE)
        set (PLIBSYS_SHORT_MIN SHRT_MIN)
        set (PLIBSYS_SHORT_MAX SHRT_MAX)
        set (PLIBSYS_USHORT_MAX USHRT_MAX)
        set (PLIBSYS_INT_MIN INT_MIN)
        set (PLIBSYS_INT_MAX INT_MAX)
        set (PLIBSYS_UINT_MAX UINT_MAX)
        set (PLIBSYS_LONG_MIN LONG_MIN)
        set (PLIBSYS_LONG_MAX LONG_MAX)
        set (PLIBSYS_ULONG_MAX ULONG_MAX)
elseif (PLIBSYS_HAVE_VALUES_H)
        set (PLIBSYS_NEED_VALUES_H TRUE)
        set (PLIBSYS_SHORT_MIN MINSHORT)
        set (PLIBSYS_SHORT_MAX MAXSHORT)
        set (PLIBSYS_USHORT_MAX "(((pushort) P_MAXSHORT) * 2 + 1)")
        set (PLIBSYS_INT_MIN MININT)
        set (PLIBSYS_INT_MAX MAXINT)
        set (PLIBSYS_UINT_MAX "(((puint) P_MAXINT) * 2 + 1)")
        set (PLIBSYS_LONG_MIN MINLONG)
        set (PLIBSYS_LONG_MAX MAXLONG)
        set (PLIBSYS_ULONG_MAX "(((pulong) P_MAXLONG) * 2 + 1)")
endif()

if (PLIBSYS_NATIVE_WINDOWS)
        set (PLIBSYS_NEED_WINDOWS_H true)
endif()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/plibsysconfig.h.in ${CMAKE_BINARY_DIR}/plibsysconfig.h)

if (NOT PLIBSYS_NATIVE_WINDOWS)
        # Check for anonymous mmap()
        message (STATUS "Checking whether mmap has anonymous mapping")

        check_c_source_compiles (
                                 "#include <sys/types.h>
                                  #include <sys/mman.h>
                                 int main () {
                                        mmap (0, 1024, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
                                        return 0;
                                 }"
                                 PLIBSYS_MMAP_HAS_MAP_ANON
                                )

        check_c_source_compiles (
                                 "#include <sys/types.h>
                                  #include <sys/mman.h>
                                 int main () {
                                        mmap (0, 1024, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
                                        return 0;
                                 }"
                                 PLIBSYS_MMAP_HAS_MAP_ANONYMOUS
                                )

        if (PLIBSYS_MMAP_HAS_MAP_ANONYMOUS OR PLIBSYS_MMAP_HAS_MAP_ANON)
                message (STATUS "Checking whether mmap has anonymous mapping - yes")
        else()
                message (STATUS "Checking whether mmap has anonymous mapping - no")
        endif()

        if (PLIBSYS_MMAP_HAS_MAP_ANONYMOUS)
                list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_MMAP_HAS_MAP_ANONYMOUS)
        elseif (PLIBSYS_MMAP_HAS_MAP_ANON)
                list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_MMAP_HAS_MAP_ANON)
        endif()

        # Check for clock_nanosleep() call
        message (STATUS "Checking whether clock_nanosleep() presents")

        check_c_source_compiles (
                                 "#include <time.h>
                                 int main () {
                                        struct timespec time_sp = {0, 500000000L};
                                        clock_nanosleep (CLOCK_MONOTONIC, 0, &time_sp, NULL);
                                        return 0;
                                 }"
                                 PLIBSYS_HAS_CLOCKNANOSLEEP
                                )

        if (PLIBSYS_HAS_CLOCKNANOSLEEP)
                message (STATUS "Checking whether clock_nanosleep() presents - yes")
        else()
                message (STATUS "Checking whether clock_nanosleep() presents - no")
        endif()

        # Check for nanosleep() call
        message (STATUS "Checking whether nanosleep() presents")

        check_c_source_compiles (
                                 "#include <time.h>
                                 int main () {
                                        struct timespec time_sp = {0, 500000000L};
                                        nanosleep (&time_sp, NULL);
                                        return 0;
                                 }"
                                 PLIBSYS_HAS_NANOSLEEP
                                )

        if (PLIBSYS_HAS_NANOSLEEP)
                message (STATUS "Checking whether nanosleep() presents - yes")
                list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_HAS_NANOSLEEP)
        else()
                message (STATUS "Checking whether nanosleep() presents - no")
        endif()

        # Prefere clock_nanosleep() over nanosleep() for power consumption
        if (PLIBSYS_HAS_CLOCKNANOSLEEP)
                list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_HAS_CLOCKNANOSLEEP)
        elseif(PLIBSYS_HAS_NANOSLEEP)
                list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_HAS_NANOSLEEP)
        endif()

        # Check for getaddrinfo() call
        message (STATUS "Checking whether getaddrinfo() presents")

        check_c_source_compiles (
                                 "#include <sys/socket.h>
                                  #include <netdb.h>
                                 int main () {
                                        getaddrinfo (0, 0, 0, 0);
                                        freeaddrinfo (0);

                                        return 0;
                                 }"
                                 PLIBSYS_HAS_GETADDRINFO
                                )

        if (PLIBSYS_HAS_GETADDRINFO)
                message (STATUS "Checking whether getaddrinfo() presents - yes")
                list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_HAS_GETADDRINFO)
        else()
                message (STATUS "Checking whether getaddrinfo() presents - no")
        endif()
endif()

if (NOT PLIBSYS_RWLOCK_MODEL)
        set (PLIBSYS_RWLOCK_MODEL ${PLIBSYS_THREAD_MODEL})
else()
        if (NOT PLIBSYS_RWLOCK_MODEL STREQUAL general AND
            NOT PLIBSYS_RWLOCK_MODEL STREQUAL none)
                message (WARNING "It's not recommended to mix threading and read-write lock models")
        endif()
endif()

if (PLIBSYS_THREAD_MODEL STREQUAL posix)
        # Some systems only need the difinition to be available
        if (PLIBSYS_TARGET_OS MATCHES "darwin|aix")
                set (PLIBSYS_SCHED_CHECK "defined (_POSIX_THREAD_PRIORITY_SCHEDULING)")
        elseif (PLIBSYS_TARGET_OS STREQUAL android)
                set (PLIBSYS_SCHED_CHECK "0")
        else()
                set (PLIBSYS_SCHED_CHECK "defined (_POSIX_THREAD_PRIORITY_SCHEDULING) && (_POSIX_THREAD_PRIORITY_SCHEDULING - 0 >= 0)")
        endif()

        # Check for thread scheduling
        message (STATUS "Checking whether POSIX thread scheduling presents")

        check_c_source_compiles (
                                 "#include <unistd.h>
                                  #include <pthread.h>
                                  #include <sched.h>

                                 int main () {
                                        #if ${PLIBSYS_SCHED_CHECK}
                                          sched_get_priority_min (0);
                                          sched_get_priority_max (0);
                                        #else
                                          stop_compile_here
                                        #endif
                                        return 0;
                                 }"
                                 PLIBSYS_HAS_POSIX_SCHEDULING
                                )

        if (PLIBSYS_HAS_POSIX_SCHEDULING)
                message (STATUS "Checking whether POSIX thread scheduling presents - yes")
                list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_HAS_POSIX_SCHEDULING)
        else()
                message (STATUS "Checking whether POSIX thread scheduling presents - no")
        endif()

        # Check for thread stack size
        message (STATUS "Checking whether POSIX thread stack size is supported")

        check_c_source_compiles (
                                 "#include <pthread.h>

                                 int main () {
                                        pthread_attr_t attr;

                                        pthread_attr_setstacksize (&attr, 0);
                                        return 0;
                                 }"
                                 PLIBSYS_HAS_POSIX_STACKSIZE
                                )

        if (PLIBSYS_HAS_POSIX_STACKSIZE)
                message (STATUS "Checking whether POSIX thread stack size is supported - yes")
                list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_HAS_POSIX_STACKSIZE)
        else()
                message (STATUS "Checking whether POSIX thread stack size is supported - no")
        endif()
endif()

# Some platforms may have headers, but lack actual implementation,
# thus we can let platform to override read-write lock model with
# general implementation
if (PLIBSYS_THREAD_MODEL STREQUAL posix AND
    PLIBSYS_RWLOCK_MODEL STREQUAL posix)
        # Check for read-write lock support
        message (STATUS "Checking whether POSIX read-write locks are supported")

        check_c_source_compiles (
                                 "#include <pthread.h>

                                 int main () {
                                        pthread_rwlock_t rwl;

                                        pthread_rwlock_init (&rwl, 0);
                                        pthread_rwlock_destroy (&rwl);
                                        return 0;
                                 }"
                                 PLIBSYS_HAS_POSIX_RWLOCK
                                )

        if (PLIBSYS_HAS_POSIX_RWLOCK)
                message (STATUS "Checking whether POSIX read-write locks are supported - yes")
        else()
                message (STATUS "Checking whether POSIX read-write locks are supported - no")
                set (PLIBSYS_RWLOCK_MODEL "general")
        endif()
endif()

list (APPEND PLIBSYS_PLATFORM_SRCS prwlock-${PLIBSYS_RWLOCK_MODEL}.c)

if (PLIBSYS_NATIVE_WINDOWS)
        set (PLIBSYS_SOCKET_INCLUDES "#include <winsock2.h>
                                      #include <ws2tcpip.h>
                                      #include <windows.h>")
else()
        set (PLIBSYS_SOCKET_INCLUDES "#include <sys/types.h>
                                      #include <sys/socket.h>
                                      #include <netinet/in.h>")
endif()

# Check for socklen_t definition
message (STATUS "Checking whether socklen_t is defined")

check_c_source_compiles (
                         "${PLIBSYS_SOCKET_INCLUDES}
                          int main () {
                                socklen_t len = sizeof (socklen_t);
                                return len > 0 ? 0 : -1;
                          }"
                          PLIBSYS_HAS_SOCKLEN_T
                        )

if (PLIBSYS_HAS_SOCKLEN_T)
        message (STATUS "Checking whether socklen_t is defined - yes")
        list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_HAS_SOCKLEN_T)
else()
        message (STATUS "Checking whether socklen_t is defined - no")
endif()

# Check for sockaddr_storage structure
message (STATUS "Checking whether struct sockaddr_storage is defined")

check_c_source_compiles (
                         "${PLIBSYS_SOCKET_INCLUDES}
                          int main () {
                                struct sockaddr_storage sock_addr;
                                sock_addr.ss_family = AF_INET;

                                return 0;
                          }"
                          PLIBSYS_HAS_SOCKADDR_STORAGE
                        )

if (PLIBSYS_HAS_SOCKADDR_STORAGE)
        message (STATUS "Checking whether struct sockaddr_storage is defined - yes")
        list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_HAS_SOCKADDR_STORAGE)
else()
        message (STATUS "Checking whether struct sockaddr_storage is defined - no")
endif()

# Check sa_len field in struct sockaddr
message (STATUS "Checking whether struct sockaddr has sa_len")

check_c_source_compiles (
                         "${PLIBSYS_SOCKET_INCLUDES}
                          int main () {
                                struct sockaddr sock_addr;
                                sock_addr.sa_len = 0;

                                return 0;
                          }"
                          PLIBSYS_SOCKADDR_HAS_SA_LEN
                        )

if (PLIBSYS_SOCKADDR_HAS_SA_LEN)
        message (STATUS "Checking whether struct sockaddr has sa_len - yes")
        list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_SOCKADDR_HAS_SA_LEN)
else()
        message (STATUS "Checking whether struct sockaddr has sa_len - no")
endif()

# Check sin6_scope_id field in struct sockaddr_in6
message (STATUS "Checking whether struct sockaddr_in6 has sin6_scope_id")

check_c_source_compiles (
                         "${PLIBSYS_SOCKET_INCLUDES}
                          int main () {
                                struct sockaddr_in6 sock_addr;
                                sock_addr.sin6_scope_id = 0;

                                return 0;
                          }"
                          PLIBSYS_SOCKADDR_IN6_HAS_SCOPEID
                        )

if (PLIBSYS_SOCKADDR_IN6_HAS_SCOPEID)
        message (STATUS "Checking whether struct sockaddr_in6 has sin6_scope_id - yes")
        list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_SOCKADDR_IN6_HAS_SCOPEID)
else()
        message (STATUS "Checking whether struct sockaddr_in6 has sin6_scope_id - no")
endif()

# Check sin6_flowinfo field in struct sockaddr_in6
message (STATUS "Checking whether struct sockaddr_in6 has sin6_flowinfo")

check_c_source_compiles (
                         "${PLIBSYS_SOCKET_INCLUDES}
                          int main () {
                                struct sockaddr_in6 sock_addr;
                                sock_addr.sin6_flowinfo = 0;

                                return 0;
                          }"
                          PLIBSYS_SOCKADDR_IN6_HAS_FLOWINFO
                        )

if (PLIBSYS_SOCKADDR_IN6_HAS_FLOWINFO)
        message (STATUS "Checking whether struct sockaddr_in6 has sin6_flowinfo - yes")
        list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_SOCKADDR_IN6_HAS_FLOWINFO)
else()
        message (STATUS "Checking whether struct sockaddr_in6 has sin6_flowinfo - no")
endif()

# Check sa_family_t type size in struct sockaddr
set (CMAKE_EXTRA_INCLUDE_FILES_OLD ${CMAKE_EXTRA_INCLUDE_FILES})

if (PLIBSYS_NATIVE_WINDOWS)
        set (CMAKE_EXTRA_INCLUDE_FILES
                winsock2.h
                ws2tcpip.h
                windows.h
        )
else()
        set (CMAKE_EXTRA_INCLUDE_FILES
                sys/types.h
                sys/socket.h
                netinet/in.h
        )
endif()

check_type_size ("((struct sockaddr *) 0)->sa_family" PLIBSYS_SIZEOF_SAFAMILY_T LANGUAGE C)

set (CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES_OLD})

# Check for lldiv() call
message (STATUS "Checking whether lldiv() presents")

check_c_source_compiles (
                         "#define __USE_ISOC99
                          #include <stdlib.h>
                          int main () {
                                lldiv_t res = lldiv (100LL, 13LL);
                                res.quot = 0;
                                res.rem = 0;

                                return 0;
                          }"
                          PLIBSYS_HAS_LLDIV
                        )

if (PLIBSYS_HAS_LLDIV)
        message (STATUS "Checking whether lldiv() presents - yes")
        list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_HAS_LLDIV)
else()
        message (STATUS "Checking whether lldiv() presents - no")
endif()

# Symbols visibility attributes
if (PLIBSYS_VISIBILITY)
        message (STATUS "Checking whether compiler supports visibility")
        plibsys_detect_visibility (PLIBSYS_VISIBILITY_CFLAGS PLIBSYS_VISIBILITY_LDFLAGS)

        if (NOT PLIBSYS_VISIBILITY_CFLAGS AND NOT PLIBSYS_VISIBILITY_LDFLAGS)
                message (STATUS "Checking whether compiler supports visibility - no")
                set (PLIBSYS_VISIBILITY OFF)
        else()
                message (STATUS "Checking whether compiler supports visibility - yes")

                if (PLIBSYS_VISIBILITY_CFLAGS)
                        list (APPEND PLIBSYS_PLATFORM_CFLAGS ${PLIBSYS_VISIBILITY_CFLAGS})
                endif()

                if (PLIBSYS_VISIBILITY_LDFLAGS)
                        list (APPEND PLIBSYS_PLATFORM_LDFLAGS ${PLIBSYS_VISIBILITY_LDFLAGS})
                endif()
        endif()
endif()

# Variable arguments

check_include_file ("stdarg.h" PLIBSYS_HAVE_STDARG_H)

if (NOT PLIBSYS_HAVE_STDARG_H)
        message (FATAL_ERROR "Support for <stdarg.h> is required for target plarform, not found")
endif()

plibsys_detect_va_copy (PLIBSYS_VA_COPY)

# Restore compiler flags
set (CMAKE_REQUIRED_DEFINITIONS ${SAVED_CMAKE_REQUIRED_DEFINITIONS})
set (CMAKE_REQUIRED_LIBRARIES ${SAVED_CMAKE_REQUIRED_LIBRARIES})
set (SAVED_CMAKE_REQUIRED_FLAGS ${SAVED_CMAKE_REQUIRED_FLAGS})

# Disable useless warnings
if (MSVC)
        list (APPEND PLIBSYS_COMPILE_DEFS -D_CRT_SECURE_NO_WARNINGS)
        list (APPEND PLIBSYS_COMPILE_DEFS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

# Prepare compile definitions
list (APPEND PLIBSYS_COMPILE_DEFS -DPLIBSYS_COMPILATION)

if (PLIBSYS_PLATFORM_DEFINES)
        list (APPEND PLIBSYS_COMPILE_DEFS ${PLIBSYS_PLATFORM_DEFINES})
endif()

# Add targets
add_library (plibsys SHARED ${PLIBSYS_SRCS} ${PLIBSYS_PLATFORM_SRCS} ${PLIBSYS_PUBLIC_HDRS} ${PLIBSYS_PRIVATE_HDRS})

if (PLIBSYS_BUILD_STATIC)
        add_library (plibsysstatic STATIC ${PLIBSYS_SRCS} ${PLIBSYS_PLATFORM_SRCS} ${PLIBSYS_PUBLIC_HDRS} ${PLIBSYS_PRIVATE_HDRS})
endif()

# Add include directories
if (COMMAND target_include_directories)
        target_include_directories (plibsys PUBLIC ${PLIBSYS_INCLUDE_DIRS})

        if (PLIBSYS_BUILD_STATIC)
                target_include_directories (plibsysstatic PUBLIC ${PLIBSYS_INCLUDE_DIRS})
        endif()
else()
        include_directories (${PLIBSYS_INCLUDE_DIRS})
endif()

# Add compile definitions
if (COMMAND target_compile_definitions)
        target_compile_definitions (plibsys PRIVATE ${PLIBSYS_COMPILE_DEFS})

        if (PLIBSYS_BUILD_STATIC)
                target_compile_definitions (plibsysstatic PRIVATE ${PLIBSYS_COMPILE_DEFS})
        endif()
else()
        add_definitions (${PLIBSYS_COMPILE_DEFS})
endif()

set_target_properties (plibsys PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plibsys PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plibsys PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

if (NOT PLIBSYS_TARGET_OS STREQUAL os2 AND NOT PLIBSYS_TARGET_OS STREQUAL amigaos)
        set_target_properties (plibsys PROPERTIES SOVERSION ${PLIBSYS_SOVERSION})
endif()

if (PLIBSYS_BUILD_STATIC)
        set_target_properties (plibsysstatic PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

        if (NOT PLIBSYS_TARGET_OS STREQUAL os2)
                set_target_properties (plibsysstatic PROPERTIES SOVERSION ${PLIBSYS_SOVERSION})
        endif()
endif()

if (PLIBSYS_PLATFORM_CFLAGS)
        foreach (PLATFORM_CFLAG ${PLIBSYS_PLATFORM_CFLAGS})
                set (PLIBSYS_PLATFORM_CFLAGS_STR "${PLIBSYS_PLATFORM_CFLAGS_STR} ${PLATFORM_CFLAG}")
        endforeach()

        set_target_properties (plibsys PROPERTIES COMPILE_FLAGS "${PLIBSYS_PLATFORM_CFLAGS_STR}")

        if (PLIBSYS_BUILD_STATIC)
                set_target_properties (plibsysstatic PROPERTIES COMPILE_FLAGS "${PLIBSYS_PLATFORM_CFLAGS_STR}")
        endif()
endif()

if (PLIBSYS_PLATFORM_LDFLAGS)
        foreach (PLATFORM_LDFLAG ${PLIBSYS_PLATFORM_LDFLAGS})
                set (PLIBSYS_PLATFORM_LDFLAGS_STR "${PLIBSYS_PLATFORM_LDFLAGS_STR} ${PLATFORM_LDFLAG}")
        endforeach()

        set_target_properties (plibsys PROPERTIES LINK_FLAGS "${PLIBSYS_PLATFORM_LDFLAGS_STR}")

        if (PLIBSYS_BUILD_STATIC)
                set_target_properties (plibsysstatic PROPERTIES LINK_FLAGS "${PLIBSYS_PLATFORM_LDFLAGS_STR}")
        endif()
endif()

target_link_libraries (plibsys ${PLIBSYS_PLATFORM_LINK_LIBRARIES})

if (PLIBSYS_BUILD_STATIC)
        target_link_libraries (plibsysstatic ${PLIBSYS_PLATFORM_LINK_LIBRARIES})
endif()

if (PLIBSYS_BUILD_STATIC)
        set (PLIBSYS_INSTALL_TARGETS plibsys plibsysstatic)
else()
        set (PLIBSYS_INSTALL_TARGETS plibsys)
endif()

if (PLIBSYS_NATIVE_WINDOWS)
        install (TARGETS ${PLIBSYS_INSTALL_TARGETS}
                DESTINATION lib
                RUNTIME DESTINATION lib
                COMPONENT Core
        )

        if (NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
                set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS TRUE)
        endif()

        set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
        include (InstallRequiredSystemLibraries)

        install (PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
                DESTINATION lib
                COMPONENT Core
        )
endif()

# Prepare installation dirs
if (NOT CMAKE_INSTALL_LIBDIR)
        set (CMAKE_INSTALL_LIBDIR "lib")
endif()

if (NOT CMAKE_INSTALL_INCLUDEDIR)
        set (CMAKE_INSTALL_INCLUDEDIR "include")
endif()

install (TARGETS ${PLIBSYS_INSTALL_TARGETS} EXPORT plibsys-targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Core
)
install (TARGETS ${PLIBSYS_INSTALL_TARGETS}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Core
)
install (FILES
        ${PLIBSYS_PUBLIC_HDRS}
        ${CMAKE_BINARY_DIR}/plibsysconfig.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/plibsys
        COMPONENT Core
)

if (PLIBSYS_VA_COPY)
        set(PLIBSYS_VA_COPY_STATUS "YES")
else()
        set(PLIBSYS_VA_COPY_STATUS "NO")
endif()

if (PLIBSYS_OS_BITS STREQUAL "universal")
    set (PLIBSYS_ADDRESS_MODEL "universal")
else()
    set (PLIBSYS_ADDRESS_MODEL "${PLIBSYS_OS_BITS} bit")
endif()

# Print summary
SET (PLIBSYS_SUMMARY
"
        == Build configuration ==

        Platfrom:               ${PLIBSYS_TARGET_OS}
        Compiler:               ${PLIBSYS_C_COMPILER}
        Address model:          ${PLIBSYS_ADDRESS_MODEL}

        Thread model:           ${PLIBSYS_THREAD_MODEL}
        RW lock model:          ${PLIBSYS_RWLOCK_MODEL}
        IPC model:              ${PLIBSYS_IPC_MODEL}
        DIR model:              ${PLIBSYS_DIR_MODEL}
        Library loader model:   ${PLIBSYS_LIBRARYLOADER_MODEL}
        Time profiler model:    ${PLIBSYS_TIME_PROFILER_MODEL}
        Atomic model:           ${PLIBSYS_ATOMIC_MODEL}

        Platform defines:       ${PLIBSYS_PLATFORM_DEFINES}
        Platform CFLAGS:        ${PLIBSYS_PLATFORM_CFLAGS}
        Platform LDFLAGS:       ${PLIBSYS_PLATFORM_LDFLAGS}
        Platform libraries:     ${PLIBSYS_PLATFORM_LINK_LIBRARIES}

        Build static library:   ${PLIBSYS_BUILD_STATIC}
        Build tests:            ${PLIBSYS_TESTS}
        Coverage support:       ${PLIBSYS_COVERAGE}
        Visibility:             ${PLIBSYS_VISIBILITY}

        va_copy availability:   ${PLIBSYS_VA_COPY_STATUS}

")

message ("${PLIBSYS_SUMMARY}")
