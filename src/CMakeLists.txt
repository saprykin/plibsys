#Try to detect target platform
string (TOLOWER ${CMAKE_SYSTEM_NAME} PLIB_TARGET_OS)

if (${CMAKE_C_COMPILER_ID} STREQUAL GNU)
	set (PLIB_C_COMPILER gcc)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
	set (PLIB_C_COMPILER msvc)
else()
	string (TOLOWER ${CMAKE_C_COMPILER_ID} PLIB_C_COMPILER)
endif()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(PLIB_ARCH_BITS 64)
else()
	set(PLIB_ARCH_BITS 32)
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "[i*86, x86_64]")
	set (PLIB_PROCESSOR_ARCH x86)
else()
	set (PLIB_PROCESSOR_ARCH ${CMAKE_SYSTEM_PROCESSOR MATCHES})
endif()

if (PLIB_TARGET_OS STREQUAL windows)
	set (PLIB_TARGET_PLATFORM win${PLIB_ARCH_BITS})
elseif (PLIB_TARGET_OS STREQUAL qnx)
	if (PLIB_C_COMPILER STREQUAL gcc)
		set (PLIB_C_COMPILER qcc)
	endif()

	set (PLIB_TARGET_PLATFORM qnx-${PLIB_PROCESSOR_ARCH})
else()
	set (PLIB_TARGET_PLATFORM ${PLIB_TARGET_OS}-${PLIB_PROCESSOR_ARCH})
endif()

set (PLIB_TARGET_PLATFORM ${PLIB_TARGET_PLATFORM}-${PLIB_C_COMPILER})

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/")
	message (FATAL_ERROR "PLib doesn't support unknown platform ${PLIB_TARGET_PLATFORM}")
endif()

include (${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/platform.cmake)

include_directories(
	${CMAKE_BINARY_DIR}
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}
)

set(PLIB_PUBLIC_HDRS
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

set(PLIB_PRIVATE_HDRS
	${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/config.h
	${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/plibconfig.h
)

set(PLIB_SRCS
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

set(PLIB_PLATFORM_SRCS
	pcondvariable-${PLIB_THREAD_MODEL}.c
	pmutex-${PLIB_THREAD_MODEL}.c
	psemaphore-${PLIB_IPC_MODEL}.c
	pshm-${PLIB_IPC_MODEL}.c
	puthread-${PLIB_THREAD_MODEL}.c
)

add_library(plib SHARED ${PLIB_SRCS} ${PLIB_PLATFORM_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})
add_library(plibstatic STATIC ${PLIB_SRCS} ${PLIB_PLATFORM_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})
add_definitions(-DPLIB_COMPILATION ${PLIB_PLATFORM_DEFINES})

if (PLIB_C_COMPILER STREQUAL msvc)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set_target_properties(plib PROPERTIES SOVERSION ${PLIB_VERSION})
set_target_properties(plibstatic PROPERTIES SOVERSION ${PLIB_VERSION})

target_link_libraries(plib ${PLIB_PLATFORM_LINK_LIBRARIES})
target_link_libraries(plibstatic ${PLIB_PLATFORM_LINK_LIBRARIES})

install(TARGETS plib plibstatic
	DESTINATION lib
    	ARCHIVE DESTINATION lib
	COMPONENT Core
)
install(TARGETS plib plibstatic EXPORT plib-targets
	DESTINATION lib
    	LIBRARY DESTINATION lib
	COMPONENT Core
)

install(FILES ${PLIB_PUBLIC_HDRS} ${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/plibconfig.h
	DESTINATION include/PLib
	COMPONENT Core
)

set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
include(InstallRequiredSystemLibraries)

install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} 
	DESTINATION bin
	COMPONENT Core
)
