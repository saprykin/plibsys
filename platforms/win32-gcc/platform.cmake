set (PLIB_THREAD_MODEL win)
set (PLIB_IPC_MODEL win)
set (PLIB_TIME_PROFILER_MODEL win)

# Ensure MSVC-compatible struct packing convention is used
# What flag to depends on gcc version: gcc3 uses '-mms-bitfields', while
# gcc2 uses '-fnative-struct'.
message (STATUS "Checking for MSVC-compatible code compilation flags")
exec_program (gcc ARGS --version OUTPUT_VARIABLE CMAKE_C_COMPILER_VERSION)

if (CMAKE_C_COMPILER_VERSION MATCHES ".*2\\.[0-9].*")
	set (PLIB_PLATFORM_CFLAGS "-fnative-struct")
	message (STATUS "Checking for MSVC-compatible code compile flags -fnative-struct")
else()
	set (PLIB_PLATFORM_CFLAGS "-mms-bitfields")
	message (STATUS "Checking for MSVC-compatible code compile flags -mms-bitfields")
endif()

set (PLIB_PLATFORM_LINK_LIBRARIES ws2_32)
