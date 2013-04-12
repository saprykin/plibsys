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

add_library (plib SHARED ${PLIB_SRCS} ${PLIB_PLATFORM_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})
add_library (plibstatic STATIC ${PLIB_SRCS} ${PLIB_PLATFORM_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})
add_definitions (-DPLIB_COMPILATION ${PLIB_PLATFORM_DEFINES})

set_target_properties (plib plibstatic PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

if (PLIB_C_COMPILER STREQUAL msvc)
	add_definitions (-D_CRT_SECURE_NO_WARNINGS)
endif()

set_target_properties (plib PROPERTIES SOVERSION ${PLIB_VERSION})
set_target_properties (plibstatic PROPERTIES SOVERSION ${PLIB_VERSION})

target_link_libraries (plib ${PLIB_PLATFORM_LINK_LIBRARIES})
target_link_libraries (plibstatic ${PLIB_PLATFORM_LINK_LIBRARIES})

install (TARGETS plib plibstatic EXPORT plib-targets
	DESTINATION lib
	RUNTIME DESTINATION lib
	COMPONENT Core
)
install (TARGETS plib plibstatic
	DESTINATION lib
	ARCHIVE DESTINATION lib
	COMPONENT Core
)
install (TARGETS plib plibstatic
	DESTINATION lib
	LIBRARY DESTINATION lib
	COMPONENT Core
)

install (FILES ${PLIB_PUBLIC_HDRS} ${PROJECT_SOURCE_DIR}/platforms/${PLIB_TARGET_PLATFORM}/plibconfig.h
	DESTINATION include/PLib
	COMPONENT Core
)

set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
include (InstallRequiredSystemLibraries)

install (PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} 
	DESTINATION lib
	COMPONENT Core
)
