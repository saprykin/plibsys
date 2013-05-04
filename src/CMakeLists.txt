include (CheckCSourceCompiles)
include (${PROJECT_SOURCE_DIR}/cmake/PlatformDetect.cmake)
set (OUTPUT_DIR ${CMAKE_BINARY_DIR})

#Try to detect target platform
string (TOLOWER ${CMAKE_SYSTEM_NAME} PLIB_TARGET_OS)

plib_detect_target_platform (PLIB_TARGET_PLATFORM)
plib_detect_c_compiler (PLIB_C_COMPILER)

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/")
	message (FATAL_ERROR "PLib doesn't support unknown platform ${PLIB_TARGET_PLATFORM}")
endif()

include (${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/platform.cmake)

# Copy platform dependent files to binary dir
file (COPY
      ${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/config.h
      ${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/plibconfig.h
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
	puthread.h
)

set (PLIB_PRIVATE_HDRS
	${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/config.h
	${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/plibconfig.h
)

set (PLIB_SRCS
	patomic.c
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

set (PLIB_PLATFORM_SRCS
	pcondvariable-${PLIB_THREAD_MODEL}.c
	pmutex-${PLIB_THREAD_MODEL}.c
	psemaphore-${PLIB_IPC_MODEL}.c
	pshm-${PLIB_IPC_MODEL}.c
	puthread-${PLIB_THREAD_MODEL}.c
)

# Detect atomic operations
set (PLIB_HAS_BUILTIN_ATOMIC_OPERATIONS false)
set (PLIB_MEMORY_BARRIER_NEEDED true)

if (PLIB_C_COMPILER STREQUAL "gcc" OR
	PLIB_C_COMPILER STREQUAL "qcc")
	check_c_source_compiles (
			"int i;
			 __sync_synchronize ();
		     __sync_bool_compare_and_swap (&i, 0, 1);
		     __sync_fetch_and_add (&i, 1);"
		     PLIB_HAS_BUILTIN_ATOMIC_OPERATIONS
	)

	message (STATUS 
			 "Checking for GCC builtin atomic operations... ${PLIB_HAS_BUILTIN_ATOMIC_OPERATIONS}"
	)

	if (PLIB_HAS_BUILTIN_ATOMIC_OPERATIONS)
		set (PLIB_MEMORY_BARRIER_NEEDED true)
	else()
		if (CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "i?86")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_I486)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "sparc*")
			check_c_source_compiles (
					"main () {
							int tmp1, tmp2, tmp3;
							__asm__ __volatile__("casx [%2], %0, %1"
								: "=&r" (tmp1), "=&r" (tmp2) : "r" (&tmp3));
					}"
					PLIB_HAS_SPARCV9
			)

			message (STATUS "Checking for SPARC V9 instruction set... ${PLIB_HAS_SPARCV9}")

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
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "alpha*")
			set (PLIB_MEMORY_BARRIER_NEEDED true)
			add_definitions (-DP_ATOMIC_ALPHA)	
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_X86_64)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc*")
			set (PLIB_MEMORY_BARRIER_NEEDED true)
			add_definitions (-DP_ATOMIC_POWERPC)

			check_c_source_compiles (
					"__asm__ __volatile__ (\"1:       nop\n\"
										   \"         bne-    1b\")"
					PLIB_HAS_POWERPC_LABELS
			)

			message (STATUS
					 "Checking for PowerPC asm numeric labels... ${PLIB_HAS_POWERPC_LABELS}")

			if (PLIB_HAS_POWERPC_LABELS)
				add_definitions (-DASM_NUMERIC_LABELS)
			endif()
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "ia64")
			set (PLIB_MEMORY_BARRIER_NEEDED true)
			add_definitions (-DP_ATOMIC_IA64)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "s390" OR
				CMAKE_SYSTEM_PROCESSOR MATCHER "s390x")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_S390)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_ARM)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "crisv32*" OR
				CMAKE_SYSTEM_PROCESSOR MATCHES "etraxfs*")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_CRISV32)
		elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "cris*" OR
				CMAKE_SYSTEM_PROCESSOR MATCHES "etrax*")
			set (PLIB_MEMORY_BARRIER_NEEDED false)
			add_definitions (-DP_ATOMIC_CRIS)
		endif()
	endif()
else()
	if (PLIB_TARGET_PLATFORM MATCHES "mingw*")
		set (PLIB_MEMORY_BARRIER_NEEDED false)
	endif()
endif()

message (STATUS "Checking whether memory barrier needed... ${PLIB_MEMORY_BARRIER_NEEDED}")

if (PLIB_HAS_BUILTIN_ATOMIC_OPERATIONS)
	set (PLIB_SRCS ${PLIB_SRCS} patomic-gcc.c)
endif()

if (PLIB_MEMORY_BARRIER_NEEDED)
	add_definitions (-DP_ATOMIC_OP_MEMORY_BARRIER_NEEDED)
endif()

add_library (plib SHARED ${PLIB_SRCS} ${PLIB_PLATFORM_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})
add_library (plibstatic STATIC ${PLIB_SRCS} ${PLIB_PLATFORM_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})
add_definitions (-DPLIB_COMPILATION ${PLIB_PLATFORM_DEFINES})

set_target_properties (plib PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plib PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties (plibstatic PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

if (PLIB_C_COMPILER STREQUAL msvc)
	add_definitions (-D_CRT_SECURE_NO_WARNINGS)
endif()

set_target_properties (plib PROPERTIES SOVERSION ${PLIB_VERSION})
set_target_properties (plibstatic PROPERTIES SOVERSION ${PLIB_VERSION})

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
install (FILES ${PLIB_PUBLIC_HDRS} ${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/plibconfig.h
	DESTINATION include/PLib
	COMPONENT Core
)
