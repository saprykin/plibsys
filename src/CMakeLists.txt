set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "Installation directory for headers")

include_directories(
	${CMAKE_BINARY_DIR}
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/config/win32
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
	${PROJECT_SOURCE_DIR}/config/win32/config.h
	${PROJECT_SOURCE_DIR}/config/win32/plibconfig.h
)

set(PLIB_SRCS
	patomic.c
	pcryptohash.c
	pcondvariable-win.c
	pfile.c
	pgost3411.c
	phashtable.c
	pinifile.c
	plist.c
	pmain.c
	pmem.c
	pmd5.c
	pmutex-win.c
	pprocess.c
	psha1.c
	pshmbuffer.c
	psemaphore.c
	psemaphore-win.c
	pshm.c
	pshm-win.c
	psocket.c
	psocketaddress.c
	pstring.c
	puthread.c
	puthread-win.c	
)

add_library(plib SHARED ${PLIB_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})
add_library(plibstatic STATIC ${PLIB_SRCS} ${PLIB_PUBLIC_HDRS} ${PLIB_PRIVATE_HDRS})
add_definitions(-DPLIB_COMPILATION)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set_target_properties(plib PROPERTIES SOVERSION ${VERSION})
set_target_properties(plibstatic PROPERTIES SOVERSION ${VERSION})

target_link_libraries(plib ws2_32)
target_link_libraries(plibstatic ws2_32)

install(TARGETS plib plibstatic
    	RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    	ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    	LIBRARY DESTINATION ${INSTALL_LIB_DIR})

install(FILES ${PLIB_PUBLIC_HDRS} ${PROJECT_SOURCE_DIR}/config/win32/plibconfig.h
	DESTINATION ${INSTALL_INC_DIR})
