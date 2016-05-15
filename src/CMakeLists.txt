include (CheckCSourceCompiles)
include (CheckTypeSize)
include (CheckIncludeFile)
include (TestBigEndian)
include (${PROJECT_SOURCE_DIR}/cmake/PlatformDetect.cmake)
set (OUTPUT_DIR ${CMAKE_BINARY_DIR})

# Try to detect target platform
plibsys_detect_target_platform (PLIBSYS_TARGET_PLATFORM)
plibsys_detect_c_compiler (PLIBSYS_C_COMPILER)
plibsys_detect_target_os (PLIBSYS_TARGET_OS)
plibsys_detect_os_bits (PLIBSYS_OS_BITS)

if (PLIBSYS_COVERAGE)
	if (PLIBSYS_C_COMPILER MATCHES "gcc|clang")
		set (CMAKE_BUILD_TYPE "Debug")
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -coverage")
	endif()
endif()

# CMP0042, see http://www.cmake.org/Wiki/CMake_RPATH_handling
if (PLIBSYS_TARGET_OS STREQUAL darwin)
	set (CMAKE_MACOSX_RPATH TRUE)
	set (CMAKE_SKIP_BUILD_RPATH FALSE)

	# Fix runtime paths on OS X 10.5 and less
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
	message (FATAL_ERROR "PLibSYS doesn't support unknown platform ${PLIBSYS_TARGET_PLATFORM}")
endif()

include (${PROJECT_SOURCE_DIR}/platforms/${PLIBSYS_TARGET_PLATFORM}/platform.cmake)

set (PLIBSYS_INCLUDE_DIRS
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_BINARY_DIR}
)

include_directories (
	${CMAKE_BINARY_DIR}
	${PROJECT_SOURCE_DIR}
	${PLIBSYS_INCLUDE_DIRS}
)

set (PLIBSYS_PUBLIC_HDRS
	patomic.h
	ptypes.h
	pmacros.h
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
	psemaphore.h
	pshm.h
	pshmbuffer.h
	psocket.h
	psocketaddress.h
	pspinlock.h
	pstring.h
	ptimeprofiler.h
	ptree.h
	puthread.h
)

set (PLIBSYS_PRIVATE_HDRS
	pcryptohash-gost3411.h
	pcryptohash-md5.h
	pcryptohash-sha1.h
	plibsys-private.h
	ptree-avl.h
	ptree-bst.h
	ptree-rb.h
	${CMAKE_BINARY_DIR}/plibsysconfig.h
)

set (PLIBSYS_SRCS
	pcryptohash.c
	pcryptohash-gost3411.c
	pcryptohash-md5.c
	pcryptohash-sha1.c
	pdir.c
	perror.c
	pfile.c
	phashtable.c
	pinifile.c
	pipc.c
	plibraryloader.c
	plist.c
	pmain.c
	pmem.c
	pprocess.c
	pshmbuffer.c
	psocket.c
	psocketaddress.c
	pstring.c
	ptree.c
	ptree-avl.c
	ptree-bst.c
	ptree-rb.c
	puthread.c
)

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

set (PLIBSYS_PLATFORM_SRCS
	pcondvariable-${PLIBSYS_THREAD_MODEL}.c
	pmutex-${PLIBSYS_THREAD_MODEL}.c
	psemaphore-${PLIBSYS_IPC_MODEL}.c
	pshm-${PLIBSYS_IPC_MODEL}.c
	puthread-${PLIBSYS_THREAD_MODEL}.c
	ptimeprofiler-${PLIBSYS_TIME_PROFILER_MODEL}.c
	pdir-${PLIBSYS_DIR_MODEL}.c
)

set (CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS} ${PLIBSYS_PLATFORM_DEFINES}")
set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${PLIBSYS_PLATFORM_LINK_LIBRARIES})

if ((PLIBSYS_TARGET_OS STREQUAL windows) AND NOT (PLIBSYS_TARGET_OS STREQUAL cygwin)
				      	 AND NOT (PLIBSYS_TARGET_OS STREQUAL msys))
	check_c_source_compiles (
				 "#include <windows.h>
				 int main () {
					(void) GetTickCount64 ();
					return 0;
				 }"
				 PLIBSYS_HAS_GETTICKCOUNT_64
	)

	check_c_source_compiles (
				 "#include <windows.h>
				 int main () {
					CONDITION_VARIABLE cv;
					InitializeConditionVariable (&cv);
					return 0;
				 }"
				 PLIBSYS_HAS_VISTA_CV
	)

	if (PLIBSYS_HAS_GETTICKCOUNT_64)
		add_definitions (-DPLIBSYS_HAS_GETTICKCOUNT_64)
	endif()

	if (PLIBSYS_HAS_VISTA_CV)
		add_definitions (-DPLIBSYS_HAS_VISTA_CV)
	endif()

	if (PLIBSYS_HAS_VISTA_CV OR PLIBSYS_HAS_GETTICKCOUNT_64)
		add_definitions (-D_WIN32_WINNT=0x0600)
		add_definitions (-DNTDDI_VERSION=0x06000000)
	else()
		add_definitions (-D_WIN32_WINNT=0x0501)
		add_definitions (-DNTDDI_VERSION=0x05010000)
	endif()
endif()

set (PLIBSYS_ATOMIC_LOCK_FREE no)

if (NOT PLIBSYS_ATOMIC_MODEL)
	message (STATUS "Checking for lock-free atomic intrinsics")

	if (NOT PLIBSYS_C_COMPILER STREQUAL clang)
		# GCC __atomic* intrinsics
		check_c_source_compiles (
					 "int main () {
					  int i, tmp_int = 0;
					  __atomic_store_4 (&i, 0, __ATOMIC_SEQ_CST);
					  __atomic_compare_exchange_n (&i, &tmp_int, 1, 0,
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
					  __sync_bool_compare_and_swap (&i, 0, 1);
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

set (PLIBSYS_SRCS
	patomic-${PLIBSYS_ATOMIC_MODEL}.c
	pspinlock-${PLIBSYS_ATOMIC_MODEL}.c
	${PLIBSYS_SRCS}
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

check_type_size ("size_t" PLIBSYS_SIZEOF_SIZE_T)
check_type_size ("long" PLIBSYS_SIZEOF_LONG)

if ((PLIBSYS_TARGET_OS STREQUAL windows) AND NOT (PLIBSYS_TARGET_OS STREQUAL cygwin)
				         AND NOT (PLIBSYS_TARGET_OS STREQUAL msys))
	set (PLIBSYS_NEED_WINDOWS_H true)
endif()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/plibsysconfig.h.in ${CMAKE_BINARY_DIR}/plibsysconfig.h)

if (NOT PLIBSYS_TARGET_OS STREQUAL windows OR PLIBSYS_TARGET_OS STREQUAL cygwin
					   OR PLIBSYS_TARGET_OS STREQUAL msys)
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
		add_definitions (-DPLIBSYS_MMAP_HAS_MAP_ANONYMOUS)
	elseif (PLIBSYS_MMAP_HAS_MAP_ANON)
		add_definitions (-DPLIBSYS_MMAP_HAS_MAP_ANON)
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
		add_definitions (-DPLIBSYS_HAS_NANOSLEEP)
	else()
		message (STATUS "Checking whether nanosleep() presents - no")
	endif()
endif()

if (PLIBSYS_THREAD_MODEL STREQUAL posix)
	if (PLIBSYS_TARGET_OS STREQUAL darwin)
		add_definitions (-DPLIBSYS_HAS_POSIX_SCHEDULING)
	else()
		# Check for thread scheduling
		message (STATUS "Checking whether POSIX thread scheduling presents")

		check_c_source_compiles (
					 "#include <unistd.h>
					  #include <sched.h>

					 int main () {
					 	#if defined (_POSIX_THREAD_PRIORITY_SCHEDULING) && (_POSIX_THREAD_PRIORITY_SCHEDULING - 0 >= 0)
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
			add_definitions (-DPLIBSYS_HAS_POSIX_SCHEDULING)
		else()
			message (STATUS "Checking whether POSIX thread scheduling presents - no")
		endif()
	endif()
endif()

if ((PLIBSYS_TARGET_OS STREQUAL windows) AND NOT (PLIBSYS_TARGET_OS STREQUAL cygwin)
				         AND NOT (PLIBSYS_TARGET_OS STREQUAL msys))
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
	add_definitions (-DPLIBSYS_HAS_SOCKLEN_T)
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
	add_definitions (-DPLIBSYS_HAS_SOCKADDR_STORAGE)
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
	add_definitions (-DPLIBSYS_SOCKADDR_HAS_SA_LEN)
else()
	message (STATUS "Checking whether struct sockaddr has sa_len - no")
endif()

add_library (plibsys SHARED ${PLIBSYS_SRCS} ${PLIBSYS_PLATFORM_SRCS} ${PLIBSYS_PUBLIC_HDRS} ${PLIBSYS_PRIVATE_HDRS})

if (PLIBSYS_BUILD_STATIC)
	add_library (plibsysstatic STATIC ${PLIBSYS_SRCS} ${PLIBSYS_PLATFORM_SRCS} ${PLIBSYS_PUBLIC_HDRS} ${PLIBSYS_PRIVATE_HDRS})
endif()

add_definitions (-DPLIBSYS_COMPILATION)

if (PLIBSYS_PLATFORM_DEFINES)
	add_definitions (${PLIBSYS_PLATFORM_DEFINES})
endif()

set_target_properties (plibsys PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plibsys PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plibsys PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plibsys PROPERTIES SOVERSION ${PLIBSYS_VERSION})

if (PLIBSYS_BUILD_STATIC)
	set_target_properties (plibsysstatic PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
	set_target_properties (plibsysstatic PROPERTIES SOVERSION ${PLIBSYS_VERSION})
endif()

if (PLIBSYS_PLATFORM_CFLAGS)
	set_target_properties (plibsys PROPERTIES COMPILE_FLAGS "${PLIBSYS_PLATFORM_CFLAGS}")

	if (PLIBSYS_BUILD_STATIC)
		set_target_properties (plibsysstatic PROPERTIES COMPILE_FLAGS "${PLIBSYS_PLATFORM_CFLAGS}")
	endif()
endif()

if (PLIBSYS_PLATFORM_LDFLAGS)
	set_target_properties (plibsys PROPERTIES LINK_FLAGS "${PLIBSYS_PLATFORM_LDFLAGS}")

	if (PLIBSYS_BUILD_STATIC)
		set_target_properties (plibsysstatic PROPERTIES LINK_FLAGS "${PLIBSYS_PLATFORM_LDFLAGS}")
	endif()
endif()

if (MSVC)
	add_definitions (-D_CRT_SECURE_NO_WARNINGS)
	add_definitions (-D_WINSOCK_DEPRECATED_NO_WARNINGS)
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

if ((PLIBSYS_TARGET_OS STREQUAL windows) AND NOT (PLIBSYS_TARGET_OS STREQUAL cygwin)
				         AND NOT (PLIBSYS_TARGET_OS STREQUAL msys))
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

install (TARGETS ${PLIBSYS_INSTALL_TARGETS} EXPORT plibsys-targets
	 DESTINATION lib
	 LIBRARY DESTINATION lib
	 COMPONENT Core
)
install (TARGETS ${PLIBSYS_INSTALL_TARGETS}
	 DESTINATION lib
	 ARCHIVE DESTINATION lib
	 COMPONENT Core
)
install (FILES
	${PLIBSYS_PUBLIC_HDRS}
	${CMAKE_BINARY_DIR}/plibsysconfig.h
	DESTINATION include/PLibSYS
	COMPONENT Core
)

# Print summary
SET (PLIBSYS_SUMMARY
"
	== Build configuration ==

	Platfrom:               ${PLIBSYS_TARGET_OS}
	Compiler:               ${PLIBSYS_C_COMPILER}
	Address model:          ${PLIBSYS_OS_BITS} bit

	Thread model:           ${PLIBSYS_THREAD_MODEL}
	IPC model:              ${PLIBSYS_IPC_MODEL}
	DIR model:              ${PLIBSYS_DIR_MODEL}
	Time profiler model:    ${PLIBSYS_TIME_PROFILER_MODEL}
	Atomic model:           ${PLIBSYS_ATOMIC_MODEL}

	Platform defines:       ${PLIBSYS_PLATFORM_DEFINES}
	Platform CFLAGS:        ${PLIBSYS_PLATFORM_CFLAGS}
	Platform LDFLAGS:       ${PLIBSYS_PLATFORM_LDFLAGS}
	Platform libraries:     ${PLIBSYS_PLATFORM_LINK_LIBRARIES}

	Coverage support:       ${PLIBSYS_COVERAGE}

")

message ("${PLIBSYS_SUMMARY}")
