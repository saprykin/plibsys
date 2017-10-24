set (CMAKE_SYSTEM_NAME QNX)
set (CMAKE_SYSTEM_VERSION 8.0.0)
set (TOOLCHAIN QNX)

set (CPUVARDIR $ENV{CPUVARDIR})

if (NOT CPUVARDIR)
        message (FATAL_ERROR "CPU architecture not set")
endif()

if (${CPUVARDIR} STREQUAL "armle-v7")
        set (CMAKE_SYSTEM_PROCESSOR armv7)
elseif (${CPUVARDIR} STREQUAL "x86")
        set (CMAKE_SYSTEM_PROCESSOR x86)
else()
        message (FATAL_ERROR "Unsupported CPU architecture: ${CPUVARDIR}")
endif()

set (QNX_HOST $ENV{QNX_HOST})
set (QNX_TARGET $ENV{QNX_TARGET})

if (NOT QNX_HOST)
        message (FATAL_ERROR "You must define QNX_HOST environment variable for toolchain")
endif()

if (NOT QNX_TARGET)
        message (FATAL_ERROR "You must define QNX_TARGET environment variable for toolchain")
endif()

set (CMAKE_SHARED_LIBRARY_PREFIX "lib")
set (CMAKE_SHARED_LIBRARY_SUFFIX ".so")
set (CMAKE_STATIC_LIBRARY_PREFIX "lib")
set (CMAKE_STATIC_LIBRARY_SUFFIX ".a")

if (CMAKE_HOST_WIN32)
        set (HOST_EXECUTABLE_SUFFIX ".exe")
endif()

set (CMAKE_MAKE_PROGRAM
        "${QNX_HOST}/usr/bin/make${HOST_EXECUTABLE_SUFFIX}"
        CACHE PATH "QNX make program"
)

set (CMAKE_SH
        "${QNX_HOST}/usr/bin/sh${HOST_EXECUTABLE_SUFFIX}"
        CACHE PATH "QNX shell program"
)

set (CMAKE_AR
        "${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-ar${HOST_EXECUTABLE_SUFFIX}"
        CACHE PATH "QNX ar program"
)

set (CMAKE_RANLIB
        "${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-ranlib${HOST_EXECUTABLE_SUFFIX}"
        CACHE PATH "QNX ranlib program"
)

set (CMAKE_NM
        "${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-nm${HOST_EXECUTABLE_SUFFIX}"
        CACHE PATH "QNX nm program"
)

set (CMAKE_OBJCOPY
        "${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-objcopy${HOST_EXECUTABLE_SUFFIX}"
        CACHE PATH "QNX objcopy program"
)

set (CMAKE_OBJDUMP
        "${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-objdump${HOST_EXECUTABLE_SUFFIX}"
        CACHE PATH "QNX objdump program"
)

set (CMAKE_LINKER
        "${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-ld${HOST_EXECUTABLE_SUFFIX}"
        CACHE PATH "QNX linker program"
)

set (CMAKE_STRIP
        "${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-strip${HOST_EXECUTABLE_SUFFIX}"
        CACHE PATH "QNX strip program"
)

set (CMAKE_C_COMPILER "${QNX_HOST}/usr/bin/qcc${HOST_EXECUTABLE_SUFFIX}")
set (CMAKE_CXX_COMPILER "${QNX_HOST}/usr/bin/qcc${HOST_EXECUTABLE_SUFFIX}")
set (CMAKE_ASM_COMPILER "${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-as${HOST_EXECUTABLE_SUFFIX}")

execute_process (COMMAND nto${CMAKE_SYSTEM_PROCESSOR}-gcc${HOST_EXECUTABLE_SUFFIX} --version
        OUTPUT_VARIABLE QCC_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

string (REGEX MATCH "[0-9]+.[0-9]+.[0-9]+" QCC_VERSION "${QCC_VERSION}")

set (CMAKE_C_COMPILER_VERSION ${QCC_VERSION})
set (CMAKE_CXX_COMPILER_VERSION ${QCC_VERSION})

set (BLACKBERRY_BASE_FLAGS "-D_REENTRANT -Wno-psabi -fstack-protector -fstack-protector-all")

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7")
        set (BLACKBERRY_CPU_FLAGS "-mcpu=cortex-a9 -mthumb")
endif()

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7")
        set (BLACKBERRY_QCC_FLAGS "-Vgcc_ntoarmv7le")
elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
        set (BLACKBERRY_QCC_FLAGS "-Vgcc_ntox86")
endif()

set (BLACKBERRY_FLAGS "${BLACKBERRY_QCC_FLAGS} ${BLACKBERRY_BASE_FLAGS} ${BLACKBERRY_CPU_FLAGS}")

set (CMAKE_C_FLAGS_DEBUG "${BLACKBERRY_FLAGS} -g" CACHE STRING "" FORCE)
set (CMAKE_C_FLAGS_MINSIZEREL "${BLACKBERRY_FLAGS} -O2 -fstack-protector-strong -Os" CACHE STRING "" FORCE)
set (CMAKE_C_FLAGS_RELEASE "${BLACKBERRY_FLAGS} -O2 -fstack-protector-strong -Os" CACHE STRING "" FORCE)
set (CMAKE_C_FLAGS_RELWITHDEBINFO "${BLACKBERRY_FLAGS} -O2 -g -fstack-protector-strong" CACHE STRING "" FORCE)

set (CMAKE_CXX_FLAGS_DEBUG "${BLACKBERRY_FLAGS} -lang-c++ -lstdc++ -g" CACHE STRING "" FORCE)
set (CMAKE_CXX_FLAGS_MINSIZEREL "${BLACKBERRY_FLAGS} -lang-c++ -lstdc++ -O2 -fstack-protector-strong -Os" CACHE STRING "" FORCE)
set (CMAKE_CXX_FLAGS_RELEASE "${BLACKBERRY_FLAGS} -lang-c++ -lstdc++ -O2 -fstack-protector-strong -Os" CACHE STRING "" FORCE)
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${BLACKBERRY_FLAGS} -lang-c++ -lstdc++ -O2 -g -fstack-protector-strong" CACHE STRING "" FORCE)

set (CMAKE_FIND_ROOT_PATH ${QNX_TARGET})
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# To remove full linking paths
link_directories (${QNX_TARGET}/${CPUVARDIR}/lib ${QNX_TARGET}/${CPUVARDIR}/usr/lib)

# To distinguish from QNX 
add_definitions (-D__BLACKBERRY10__)

set (CMAKE_LIBRARY_PATH
        ${QNX_TARGET}/${CPUVARDIR}/lib
        ${QNX_TARGET}/${CPUVARDIR}/usr/lib
)

set (CMAKE_INCLUDE_PATH
        ${QNX_TARGET}/usr/include/c++/${QCC_VERSION}
        ${QNX_TARGET}/usr/include/c++/${QCC_VERSION}/arm-unknown-nto-qnx8.0.0eabi
)
