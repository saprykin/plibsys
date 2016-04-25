include (CheckCSourceCompiles)
include (CheckTypeSize)
include (CheckIncludeFile)
include (TestBigEndian)
include (${PROJECT_SOURCE_DIR}/cmake/PlatformDetect.cmake)
set (OUTPUT_DIR ${CMAKE_BINARY_DIR})

# Try to detect target platform
plib_detect_target_platform (PLIB_TARGET_PLATFORM)
plib_detect_c_compiler (PLIB_C_COMPILER)
plib_detect_target_os (PLIB_TARGET_OS)
plib_detect_os_bits (PLIB_OS_BITS)

if (PLIB_COVERAGE)
	if (PLIB_C_COMPILER MATCHES "gcc|clang")
		set (CMAKE_BUILD_TYPE "Debug")
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -coverage")
	endif()
endif()

# CMP0042, see http://www.cmake.org/Wiki/CMake_RPATH_handling
if (PLIB_TARGET_OS STREQUAL darwin)
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

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/")
	message (FATAL_ERROR "PLib doesn't support unknown platform ${PLIB_TARGET_PLATFORM}")
endif()

include (${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/platform.cmake)

set (PLIB_INCLUDE_DIRS
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_BINARY_DIR}
)

include_directories (
	${CMAKE_BINARY_DIR}
	${PROJECT_SOURCE_DIR}
	${PLIB_INCLUDE_DIRS}
)

set (PLIB_PUBLIC_HDRS
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
	plib.h
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
	pstring.h
	ptimeprofiler.h
	ptree.h
	puthread.h
)

set (PLIB_PRIVATE_HDRS
	pcryptohash-gost3411.h
	pcryptohash-md5.h
	pcryptohash-sha1.h
	plib-private.h
	ptree-avl.h
	ptree-bst.h
	ptree-rb.h
	${CMAKE_BINARY_DIR}/plibconfig.h
)

set (PLIB_SRCS
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

if (PLIB_THREAD_MODEL STREQUAL "")
	set (PLIB_THREAD_MODEL none)
endif()

if (PLIB_IPC_MODEL STREQUAL "")
	set (PLIB_IPC_MODEL none)
endif()

if (PLIB_TIME_PROFILER_MODEL STREQUAL "")
	set (PLIB_TIME_PROFILER_MODEL generic)
endif()

if (PLIB_DIR_MODEL STREQUAL "")
	set (PLIB_DIR_MODEL none)
endif()

set (PLIB_PLATFORM_SRCS
	pcondvariable-${PLIB_THREAD_MODEL}.c
	pmutex-${PLIB_THREAD_MODEL}.c
	psemaphore-${PLIB_IPC_MODEL}.c
	pshm-${PLIB_IPC_MODEL}.c
	puthread-${PLIB_THREAD_MODEL}.c
	ptimeprofiler-${PLIB_TIME_PROFILER_MODEL}.c
	pdir-${PLIB_DIR_MODEL}.c
)

set (CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS} ${PLIB_PLATFORM_DEFINES}")

if ((PLIB_TARGET_OS STREQUAL windows) AND NOT (PLIB_TARGET_OS STREQUAL cygwin)
				      AND NOT (PLIB_TARGET_OS STREQUAL msys))
	check_c_source_compiles (
				 "#include <windows.h>
				 int main () {
					(void) GetTickCount64 ();
					return 0;
				 }"
				 PLIB_HAS_GETTICKCOUNT_64
	)

	check_c_source_compiles (
				 "#include <windows.h>
				 int main () {
					CONDITION_VARIABLE cv;
					InitializeConditionVariable (&cv);
					return 0;
				 }"
				 PLIB_HAS_VISTA_CV
	)

	if (PLIB_HAS_GETTICKCOUNT_64)
		add_definitions (-DPLIB_HAS_GETTICKCOUNT_64)
	endif()

	if (PLIB_HAS_VISTA_CV)
		add_definitions (-DPLIB_HAS_VISTA_CV)
	endif()

	if (PLIB_HAS_VISTA_CV OR PLIB_HAS_GETTICKCOUNT_64)
		add_definitions (-D_WIN32_WINNT=0x0600)
		add_definitions (-DNTDDI_VERSION=0x06000000)
	else()
		add_definitions (-D_WIN32_WINNT=0x0501)
		add_definitions (-DNTDDI_VERSION=0x05010000)
	endif()
endif()

set (PLIB_ATOMIC_LOCK_FREE no)

if (NOT PLIB_ATOMIC_MODEL)
	message (STATUS "Checking for lock-free atomic intrinsics")

	if (NOT PLIB_C_COMPILER STREQUAL clang)
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
					 PLIB_ATOMIC_IMPL_GCCATOMIC
		)

		if (PLIB_ATOMIC_IMPL_GCCATOMIC)
			set (PLIB_ATOMIC_LOCK_FREE yes)
			set (PLIB_ATOMIC_MODEL "c11")
		endif()
	endif()

	# GCC __sync* intinsics
	if (NOT PLIB_ATOMIC_LOCK_FREE)
		check_c_source_compiles (
					 "int main () {
					  int i;
					  __sync_synchronize ();
					  __sync_bool_compare_and_swap (&i, 0, 1);
					  __sync_fetch_and_add (&i, 1);
					  return 0;
					 }"
					 PLIB_ATOMIC_IMPL_GCCSYNC
		)

		if (PLIB_ATOMIC_IMPL_GCCSYNC)
			set (PLIB_ATOMIC_LOCK_FREE yes)
			set (PLIB_ATOMIC_MODEL "sync")
		endif()
	endif()

	if (NOT PLIB_ATOMIC_LOCK_FREE)
		if (PLIB_TARGET_OS STREQUAL windows AND (PLIB_C_COMPILER STREQUAL borland OR
							 PLIB_C_COMPILER STREQUAL watcom  OR
							 PLIB_C_COMPILER STREQUAL icc     OR
							 PLIB_C_COMPILER STREQUAL msvc))
			set (PLIB_ATOMIC_LOCK_FREE true)
			set (PLIB_ATOMIC_MODEL "win")
		else()
			set (PLIB_ATOMIC_LOCK_FREE no)
			set (PLIB_ATOMIC_MODEL "sim")
		endif()
	endif()

	if (PLIB_ATOMIC_LOCK_FREE)
		message (STATUS "Checking for lock-free atomic intrinsics - works")
	else()
		message (STATUS "Checking for lock-free atomic intrinsics - not works")
	endif()
endif()

set (PLIB_SRCS "patomic-${PLIB_ATOMIC_MODEL}" ${PLIB_SRCS})

if (EXISTS PLIB_CONFIG_FILE)
	file (RENOVE ${PLIB_CONFIG_FILE})
endif()

test_big_endian (PLIB_IS_BIGENDIAN)

check_include_file ("float.h" PLIB_HAVE_FLOAT_H)
check_include_file ("values.h" PLIB_HAVE_VALUES_H)
check_include_file ("limits.h" PLIB_HAVE_LIMITS_H)

if (PLIB_HAVE_FLOAT_H)
	set (PLIB_NEED_FLOAT_H TRUE)
	set (PLIB_FLOAT_MIN FLT_MIN)
	set (PLIB_FLOAT_MAX FLT_MAX)
	set (PLIB_DOUBLE_MIN DBL_MIN)
	set (PLIB_DOUBLE_MAX DBL_MAX)
elseif (PLIB_HAVE_VALUES_H)
	set (PLIB_NEED_VALUES_H TRUE)
	set (PLIB_FLOAT_MIN MINFLOAT)
	set (PLIB_FLOAT_MAX MAXFLOAT)
	set (PLIB_DOUBLE_MIN MINDOUBLE)
	set (PLIB_DOUBLE_MAX MAXDOUBLE)
endif()

if (PLIB_HAVE_LIMITS_H)
	set (PLIB_NEED_LIMITS_H TRUE)
	set (PLIB_SHORT_MIN SHRT_MIN)
	set (PLIB_SHORT_MAX SHRT_MAX)
	set (PLIB_USHORT_MAX USHRT_MAX)
	set (PLIB_INT_MIN INT_MIN)
	set (PLIB_INT_MAX INT_MAX)
	set (PLIB_UINT_MAX UINT_MAX)
	set (PLIB_LONG_MIN LONG_MIN)
	set (PLIB_LONG_MAX LONG_MAX)
	set (PLIB_ULONG_MAX ULONG_MAX)
elseif (PLIB_HAVE_VALUES_H)
	set (PLIB_NEED_VALUES_H TRUE)
	set (PLIB_SHORT_MIN MINSHORT)
	set (PLIB_SHORT_MAX MAXSHORT)
	set (PLIB_USHORT_MAX "(((pushort) P_MAXSHORT) * 2 + 1)")
	set (PLIB_INT_MIN MININT)
	set (PLIB_INT_MAX MAXINT)
	set (PLIB_UINT_MAX "(((puint) P_MAXINT) * 2 + 1)")
	set (PLIB_LONG_MIN MINLONG)
	set (PLIB_LONG_MAX MAXLONG)
	set (PLIB_ULONG_MAX "(((pulong) P_MAXLONG) * 2 + 1)")
endif()

check_type_size ("size_t" PLIB_SIZEOF_SIZE_T)
check_type_size ("long" PLIB_SIZEOF_LONG)

if ((PLIB_TARGET_OS STREQUAL windows) AND NOT (PLIB_TARGET_OS STREQUAL cygwin)
				      AND NOT (PLIB_TARGET_OS STREQUAL msys))
	set (PLIB_NEED_WINDOWS_H true)
endif()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/plibconfig.h.in ${CMAKE_BINARY_DIR}/plibconfig.h)

if (NOT PLIB_TARGET_OS STREQUAL windows OR PLIB_TARGET_OS STREQUAL cygwin
					OR PLIB_TARGET_OS STREQUAL msys)
	# Check for anonymous mmap()
	message (STATUS "Checking whether mmap has anonymous mapping")

	check_c_source_compiles (
				 "#include <sys/types.h>
				  #include <sys/mman.h>
				 int main () {
					mmap (0, 1024, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
					return 0;
				 }"
				 PLIB_MMAP_HAS_MAP_ANON
				)

	check_c_source_compiles (
				 "#include <sys/types.h>
				  #include <sys/mman.h>
				 int main () {
					mmap (0, 1024, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
					return 0;
				 }"
				 PLIB_MMAP_HAS_MAP_ANONYMOUS
				)

	if (PLIB_MMAP_HAS_MAP_ANONYMOUS OR PLIB_MMAP_HAS_MAP_ANON)
		message (STATUS "Checking whether mmap has anonymous mapping - yes")
	else()
		message (STATUS "Checking whether mmap has anonymous mapping - no")
	endif()

	if (PLIB_MMAP_HAS_MAP_ANONYMOUS)
		add_definitions (-DPLIB_MMAP_HAS_MAP_ANONYMOUS)
	elseif (PLIB_MMAP_HAS_MAP_ANON)
		add_definitions (-DPLIB_MMAP_HAS_MAP_ANON)
	endif()

	set (OLD_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
	set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${PLIB_PLATFORM_LINK_LIBRARIES})

	# Check for nanosleep() call
	message (STATUS "Checking whether nanosleep() presents")

	check_c_source_compiles (
				 "#include <time.h>
				 int main () {
					struct timespec time_sp = {0, 500000000L};
					nanosleep (&time_sp, NULL);
					return 0;
				 }"
				 PLIB_HAS_NANOSLEEP
				)

	if (PLIB_HAS_NANOSLEEP)
		message (STATUS "Checking whether nanosleep() presents - yes")
		add_definitions (-DPLIB_HAS_NANOSLEEP)
	else()
		message (STATUS "Checking whether nanosleep() presents - no")
	endif()

	set (CMAKE_REQUIRED_LIBRARIES ${OLD_CMAKE_REQUIRED_LIBRARIES})
endif()

if ((PLIB_TARGET_OS STREQUAL windows) AND NOT (PLIB_TARGET_OS STREQUAL cygwin)
				      AND NOT (PLIB_TARGET_OS STREQUAL msys))
	set (PLIB_SOCKET_INCLUDES "#include <winsock2.h>
				   #include <ws2tcpip.h>
				   #include <windows.h>")
else()
	set (PLIB_SOCKET_INCLUDES "#include <sys/types.h>
				   #include <sys/socket.h>
				   #include <netinet/in.h>")
endif()

# Check for socklen_t definition
message (STATUS "Checking whether socklen_t is defined")

check_c_source_compiles (
			 "${PLIB_SOCKET_INCLUDES}
			  int main () {
				socklen_t len = sizeof (socklen_t);
				return len > 0 ? 0 : -1;
			  }"
			  PLIB_HAS_SOCKLEN_T
			)

if (PLIB_HAS_SOCKLEN_T)
	message (STATUS "Checking whether socklen_t is defined - yes")
	add_definitions (-DPLIB_HAS_SOCKLEN_T)
else()
	message (STATUS "Checking whether socklen_t is defined - no")
endif()

# Check for sockaddr_storage structure
message (STATUS "Checking whether struct sockaddr_storage is defined")

check_c_source_compiles (
			 "${PLIB_SOCKET_INCLUDES}
			  int main () {
				struct sockaddr_storage sock_addr;
				sock_addr.ss_family = AF_INET;

				return 0;
			  }"
			  PLIB_HAS_SOCKADDR_STORAGE
			)

if (PLIB_HAS_SOCKADDR_STORAGE)
	message (STATUS "Checking whether struct sockaddr_storage is defined - yes")
	add_definitions (-DPLIB_HAS_SOCKADDR_STORAGE)
else()
	message (STATUS "Checking whether struct sockaddr_storage is defined - no")
endif()

# Check sa_len field in struct sockaddr
message (STATUS "Checking whether struct sockaddr has sa_len")

check_c_source_compiles (
			 "${PLIB_SOCKET_INCLUDES}
			  int main () {
				struct sockaddr sock_addr;
				sock_addr.sa_len = 0;

				return 0;
			  }"
			  PLIB_SOCKADDR_HAS_SA_LEN
			)

if (PLIB_SOCKADDR_HAS_SA_LEN)
	message (STATUS "Checking whether struct sockaddr has sa_len - yes")
	add_definitions (-DPLIB_SOCKADDR_HAS_SA_LEN)
else()
	message (STATUS "Checking whether struct sockaddr has sa_len - no")
endif()

add_library (plib SHARED ${PLIB_SRCS} ${PLIB_PLATFORM_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})

if (PLIB_BUILD_STATIC)
	add_library (plibstatic STATIC ${PLIB_SRCS} ${PLIB_PLATFORM_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})
endif()

add_definitions (-DPLIB_COMPILATION)

if (PLIB_PLATFORM_DEFINES)
	add_definitions (${PLIB_PLATFORM_DEFINES})
endif()

set_target_properties (plib PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plib PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plib PROPERTIES SOVERSION ${PLIB_VERSION})

if (PLIB_BUILD_STATIC)
	set_target_properties (plibstatic PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
	set_target_properties (plibstatic PROPERTIES SOVERSION ${PLIB_VERSION})
endif()

if (PLIB_PLATFORM_CFLAGS)
	set_target_properties (plib PROPERTIES COMPILE_FLAGS "${PLIB_PLATFORM_CFLAGS}")

	if (PLIB_BUILD_STATIC)
		set_target_properties (plibstatic PROPERTIES COMPILE_FLAGS "${PLIB_PLATFORM_CFLAGS}")
	endif()
endif()

if (PLIB_PLATFORM_LDFLAGS)
	set_target_properties (plib PROPERTIES LINK_FLAGS "${PLIB_PLATFORM_LDFLAGS}")

	if (PLIB_BUILD_STATIC)
		set_target_properties (plibstatic PROPERTIES LINK_FLAGS "${PLIB_PLATFORM_LDFLAGS}")
	endif()
endif()

if (MSVC)
	add_definitions (-D_CRT_SECURE_NO_WARNINGS)
	add_definitions (-D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

target_link_libraries (plib ${PLIB_PLATFORM_LINK_LIBRARIES})

if (PLIB_BUILD_STATIC)
	target_link_libraries (plibstatic ${PLIB_PLATFORM_LINK_LIBRARIES})
endif()

if (PLIB_BUILD_STATIC)
	set (PLIB_INSTALL_TARGETS plib plibstatic)
else()
	set (PLIB_INSTALL_TARGETS plib)
endif()

if ((PLIB_TARGET_OS STREQUAL windows) AND NOT (PLIB_TARGET_OS STREQUAL cygwin)
				      AND NOT (PLIB_TARGET_OS STREQUAL msys))
	install (TARGETS ${PLIB_INSTALL_TARGETS}
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

install (TARGETS ${PLIB_INSTALL_TARGETS} EXPORT plib-targets
	DESTINATION lib
	LIBRARY DESTINATION lib
	COMPONENT Core
)
install (TARGETS ${PLIB_INSTALL_TARGETS}
	DESTINATION lib
	ARCHIVE DESTINATION lib
	COMPONENT Core
)
install (FILES
	${PLIB_PUBLIC_HDRS}
	${CMAKE_BINARY_DIR}/plibconfig.h
	DESTINATION include/PLib
	COMPONENT Core
)

# Print summary
SET (PLIB_SUMMARY
"
	== Build configuration ==

	Platfrom:               ${PLIB_TARGET_OS}
	Compiler:               ${PLIB_C_COMPILER}
	Address model:          ${PLIB_OS_BITS} bit

	Thread model:           ${PLIB_THREAD_MODEL}
	IPC model:              ${PLIB_IPC_MODEL}
	DIR model:              ${PLIB_DIR_MODEL}
	Time profiler model:    ${PLIB_TIME_PROFILER_MODEL}
	Atomic model:           ${PLIB_ATOMIC_MODEL}

	Platform defines:       ${PLIB_PLATFORM_DEFINES}
	Platform CFLAGS:        ${PLIB_PLATFORM_CFLAGS}
	Platform LDFLAGS:       ${PLIB_PLATFORM_LDFLAGS}
	Platform libraries:     ${PLIB_PLATFORM_LINK_LIBRARIES}

	Coverage support:       ${PLIB_COVERAGE}

")

message ("${PLIB_SUMMARY}")
