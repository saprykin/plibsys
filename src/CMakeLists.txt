include (CheckCSourceCompiles)
include (CheckTypeSize)
include (CheckIncludeFile)
include (TestBigEndian)
include (${PROJECT_SOURCE_DIR}/cmake/PlatformDetect.cmake)
set (OUTPUT_DIR ${CMAKE_BINARY_DIR})
set (PLIB_CONFIG_FILE ${CMAKE_BINARY_DIR}/plibconfig.h)

#Try to detect target platform
string (TOLOWER ${CMAKE_SYSTEM_NAME} PLIB_TARGET_OS)

plib_detect_target_platform (PLIB_TARGET_PLATFORM)
plib_detect_c_compiler (PLIB_C_COMPILER)
string (TOLOWER ${CMAKE_SYSTEM_NAME} PLIB_TARGET_OS)

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/")
	message (FATAL_ERROR "PLib doesn't support unknown platform ${PLIB_TARGET_PLATFORM}")
endif()

include (${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/platform.cmake)

# Copy platform dependent files to binary dir
file (COPY
      ${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/pplatform.h
      DESTINATION ${PROJECT_BINARY_DIR})

set (PLIB_INCLUDE_DIRS
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}
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
	pfile.h
	pgost3411.h
	phashtable.h
	pinifile.h
	plib.h
	plist.h
	pmain.h
	pmd5.h
	pmem.h
	pmutex.h
	pprocess.h
	psemaphore.h
	psha1.h
	pshm.h
	pshmbuffer.h
	psocket.h
	psocketaddress.h
	pstring.h
        ptimeprofiler.h
	puthread.h
)

set (PLIB_PRIVATE_HDRS
	${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/pplatform.h
	${CMAKE_BINARY_DIR}/plibconfig.h
)

set (PLIB_SRCS
	pcryptohash.c
	pfile.c
	pgost3411.c
	phashtable.c
	pinifile.c
	plist.c
	pmain.c
	pmem.c
	pmd5.c
	pprocess.c
	psha1.c
	pshmbuffer.c
	psemaphore.c
	pshm.c
	psocket.c
	psocketaddress.c
	pstring.c
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

set (PLIB_PLATFORM_SRCS
	pcondvariable-${PLIB_THREAD_MODEL}.c
	pmutex-${PLIB_THREAD_MODEL}.c
	psemaphore-${PLIB_IPC_MODEL}.c
	pshm-${PLIB_IPC_MODEL}.c
	puthread-${PLIB_THREAD_MODEL}.c
	ptimeprofiler-${PLIB_TIME_PROFILER_MODEL}.c
)

if (PLIB_TARGET_OS STREQUAL windows)
	check_c_source_compiles (
				 "#include <windows.h>
				 int main () {
				 	(void) GetTickCount64 ();
				 	return 0;
				 }"
				 PLIB_HAS_GETTICKCOUNT_64
				)
endif()

if (PLIB_HAS_GETTICKCOUNT_64)
	add_definitions (-DPLIB_HAS_GETTICKCOUNT_64)
endif()

# Detect atomic operations
set (PLIB_MEMORY_BARRIER_NEEDED true)

if (PLIB_C_COMPILER MATCHES "gcc|qcc")
	message (STATUS "Checking for GCC builtin atomic operations")

	check_c_source_compiles (
				 "int main () {
				int i;
				__sync_synchronize ();
				__sync_bool_compare_and_swap (&i, 0, 1);
				__sync_fetch_and_add (&i, 1);
				return 0;
				}"
				PLIB_HAS_BUILTIN_ATOMIC_OPERATIONS
	)

	if (PLIB_HAS_BUILTIN_ATOMIC_OPERATIONS)
		message (STATUS "Checking for GCC builtin atomic operations - works")
	else()
		message (STATUS "Checking for GCC builtin atomic operations - not works")
	endif()
	
	if (PLIB_HAS_BUILTIN_ATOMIC_OPERATIONS)
		set (PLIB_MEMORY_BARRIER_NEEDED true)
	else()
		if (CMAKE_SYSTEM_PROCESSOR STREQUAL "i386")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "i[1-9]86")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_I486)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "sparc.*")
			message (STATUS "Checking for SPARC V9 instruction set")

			check_c_source_compiles (
					"int main () {
						int tmp1, tmp2, tmp3;
						__asm__ __volatile__(\"casx [%2], %0, %1\"
							: \"=&r\" (tmp1), \"=&r\" (tmp2) : \"r\" (&tmp3));
						return 0;
					}"
					PLIB_HAS_SPARCV9
			)

			if (PLIB_HAS_SPARCV9)
				message (STATUS "Checking for SPARC V9 instruction set - works")
			else()
				message (STATUS "Checking for SPARC V9 instruction set - not works")
			endif()

			if (PLIB_HAS_SPARCV9)
				add_definitions (-DP_ATOMIC_SPARCV9)
			else()
				message (WARNING
						 "Try to rerun configure with '-mcpu=v9' complier flag,
						 when you are using a sparc with v9 instruction set (most
						 sparcs nowadays). This will make the code for atomic
						 operations much faster. The resulting code will not run
						 on very old sparcs though.")
			endif()

			set (PLIB_MEMORY_BARRIER_NEEDED true)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "alpha.*")
			set (PLIB_MEMORY_BARRIER_NEEDED true)
			add_definitions (-DP_ATOMIC_ALPHA)	
		elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_X86_64)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc.*")
			set (PLIB_MEMORY_BARRIER_NEEDED true)
			add_definitions (-DP_ATOMIC_POWERPC)

			message (STATUS "Checking for PowerPC asm numeric labels")

			check_c_source_compiles (
					"int main () {
						__asm__ __volatile__ (\"1:       nop\n\"
								      \"         bne-    1b\");
						return 0;
					}"
					PLIB_HAS_POWERPC_LABELS
			)

			if (PLIB_HAS_POWERPC_LABELS)
				message (STATUS "Checking for PowerPC asm numeric labels - works")
			else()
				message (STATUS "Checking for PowerPC asm numeric labels - not works")
			endif()

			if (PLIB_HAS_POWERPC_LABELS)
				add_definitions (-DASM_NUMERIC_LABELS)
			endif()
		elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "ia64")
			set (PLIB_MEMORY_BARRIER_NEEDED true)
			add_definitions (-DP_ATOMIC_IA64)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "s390|s390x")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_S390)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm.*")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_ARM)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "(crisv32.*)|(etraxfs.*)")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_CRISV32)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "(cris.*)|(etrax.*)")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_CRIS)
		endif()
	endif()
else()
	if (PLIB_TARGET_OS MATCHES "(mingw.*)|(windows.*)")
		set (PLIB_MEMORY_BARRIER_NEEDED false)
	endif()
endif()

message (STATUS "Checking whether memory barrier needed")

if (PLIB_MEMORY_BARRIER_NEEDED)
	message (STATUS "Checking whether memory barrier needed - yes")
else()
	message (STATUS "Checking whether memory barrier needed - no")
endif()

if (PLIB_HAS_BUILTIN_ATOMIC_OPERATIONS)
	set (PLIB_SRCS patomic-gcc.c ${PLIB_SRCS})
else()
	set (PLIB_SRCS patomic.c ${PLIB_SRCS})
endif()

if (PLIB_MEMORY_BARRIER_NEEDED)
	add_definitions (-DP_ATOMIC_OP_MEMORY_BARRIER_NEEDED)
endif()

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

file (WRITE ${PLIB_CONFIG_FILE} "#ifndef __PLIBCONFIG_H__\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define __PLIBCONFIG_H__\n\n")

if (PLIB_TARGET_OS STREQUAL windows)
	file (APPEND ${PLIB_CONFIG_FILE} "#include <winsock2.h>\n")
	file (APPEND ${PLIB_CONFIG_FILE} "#include <windows.h>\n")
endif()

file (APPEND ${PLIB_CONFIG_FILE} "#include <pmacros.h>\n")
file (APPEND ${PLIB_CONFIG_FILE} "#include <pplatform.h>\n")

if (PLIB_NEED_FLOAT_H)
	file (APPEND ${PLIB_CONFIG_FILE} "#include <float.h>\n")
endif()

if (PLIB_NEED_LIMITS_H)
	file (APPEND ${PLIB_CONFIG_FILE} "#include <limits.h>\n")
endif()

if (PLIB_NEED_VALUES_H)
	file (APPEND ${PLIB_CONFIG_FILE} "#include <values.h>\n")
endif()

file (APPEND ${PLIB_CONFIG_FILE} "\nP_BEGIN_DECLS\n\n")

file (APPEND ${PLIB_CONFIG_FILE} "#define P_MINFLOAT	${PLIB_FLOAT_MIN}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MAXFLOAT	${PLIB_FLOAT_MAX}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MINDOUBLE	${PLIB_DOUBLE_MIN}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MAXDOUBLE	${PLIB_DOUBLE_MAX}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MINSHORT	${PLIB_SHORT_MIN}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MAXSHORT	${PLIB_SHORT_MAX}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MAXUSHORT	${PLIB_USHORT_MAX}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MININT	${PLIB_INT_MIN}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MAXINT	${PLIB_INT_MAX}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MAXUINT	${PLIB_UINT_MAX}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MINLONG	${PLIB_LONG_MIN}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MAXLONG	${PLIB_LONG_MAX}\n")
file (APPEND ${PLIB_CONFIG_FILE} "#define P_MAXULONG	${PLIB_ULONG_MAX}\n\n")

file (APPEND ${PLIB_CONFIG_FILE} "#define PLIB_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P}\n")
file (APPEND ${PLIB_CONFIG_FILE} "${PLIB_SIZEOF_SIZE_T_CODE}\n")
file (APPEND ${PLIB_CONFIG_FILE} "${PLIB_SIZEOF_LONG_CODE}\n")

if (PLIB_IS_BIGENDIAN)
	file (APPEND ${PLIB_CONFIG_FILE} "#define P_BYTE_ORDER P_BIG_ENDIAN\n")
else()
	file (APPEND ${PLIB_CONFIG_FILE} "#define P_BYTE_ORDER P_LITTLE_ENDIAN\n")
endif()

file (APPEND ${PLIB_CONFIG_FILE} "\nP_END_DECLS\n\n")
file (APPEND ${PLIB_CONFIG_FILE} "#endif /* __PLIBCONFIG_H__ */\n")

if (NOT PLIB_TARGET_OS STREQUAL windows)
	message (STATUS "Checking whether mmap has anonymous mapping")

	check_c_source_compiles (
				 "#include <sys/mman.h>
				 int main () {
				 	mmap (0, 1024, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
				 	return 0;
				 }"
				 PLIB_MMAP_HAS_MAP_ANON
				)

	check_c_source_compiles (
				 "#include <sys/mman.h>
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
endif()

add_library (plib SHARED ${PLIB_SRCS} ${PLIB_PLATFORM_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})
add_library (plibstatic STATIC ${PLIB_SRCS} ${PLIB_PLATFORM_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})
add_definitions (-DPLIB_COMPILATION)

if (PLIB_PLATFORM_DEFINES)
	add_definitions (${PLIB_PLATFORM_DEFINES})
endif()

set_target_properties (plib PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plib PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plibstatic PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

if (PLIB_C_COMPILER STREQUAL msvc)
	add_definitions (-D_CRT_SECURE_NO_WARNINGS)
endif()

set_target_properties (plib PROPERTIES SOVERSION ${PLIB_VERSION})
set_target_properties (plibstatic PROPERTIES SOVERSION ${PLIB_VERSION})

if (PLIB_PLATFORM_CFLAGS)
	set_target_properties (plib PROPERTIES COMPILE_FLAGS ${PLIB_PLATFORM_CFLAGS})
	set_target_properties (plibstatic PROPERTIES COMPILE_FLAGS ${PLIB_PLATFORM_CFLAGS})
endif()

target_link_libraries (plib ${PLIB_PLATFORM_LINK_LIBRARIES})
target_link_libraries (plibstatic ${PLIB_PLATFORM_LINK_LIBRARIES})

if (PLIB_TARGET_OS STREQUAL windows)
	install (TARGETS plib plibstatic
		DESTINATION lib
		RUNTIME DESTINATION lib
		COMPONENT Core
	)
	set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
	include (InstallRequiredSystemLibraries)
	
	install (PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} 
		DESTINATION lib
		COMPONENT Core
	)
endif()

install (TARGETS plib plibstatic EXPORT plib-targets
	DESTINATION lib
	LIBRARY DESTINATION lib
	COMPONENT Core
)
install (TARGETS plib plibstatic
	DESTINATION lib
	ARCHIVE DESTINATION lib
	COMPONENT Core
)
install (FILES
	${PLIB_PUBLIC_HDRS}
	${CMAKE_BINARY_DIR}/plibconfig.h
	${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/pplatform.h
	DESTINATION include/PLib
	COMPONENT Core
)
