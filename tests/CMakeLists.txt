project (tests CXX)
set (OUTPUT_DIR ${CMAKE_BINARY_DIR})

include (${PROJECT_SOURCE_DIR}/../cmake/PlatformDetect.cmake)
plibsys_detect_target_os (PLIBSYS_TESTS_TARGET_OS)

if (NOT PLIBSYS_TESTS_STATIC)
        find_package (Boost COMPONENTS unit_test_framework REQUIRED)
        list (APPEND PLIBSYS_TEST_INCLUDE_DIRS ${Boost_INCLUDE_DIR})
else()
        find_package (Boost COMPONENTS unit_test_framework)

        if (NOT Boost_FOUND AND NOT BOOST_ROOT)
                message (FATAL_ERROR "You must specify BOOST_ROOT variable when building static tests without proper Boost installation")
        elseif (Boost_FOUND)
                list (APPEND PLIBSYS_TEST_COMPILE_DEFS -DPLIBSYS_TESTS_STATIC)
                list (APPEND PLIBSYS_TEST_INCLUDE_DIRS ${Boost_INCLUDE_DIR})
                set (Boost_LIBRARIES "")
        else()
                list (APPEND PLIBSYS_TEST_COMPILE_DEFS -DPLIBSYS_TESTS_STATIC)
                set (Boost_LIBRARIES "")

                if (NOT IS_ABSOLUTE "${BOOST_ROOT}")
                        get_filename_component (BOOST_ROOT_ABSOLUTE "${CMAKE_CURRENT_BINARY_DIR}/${BOOST_ROOT}" ABSOLUTE)
                        set (BOOST_ROOT ${BOOST_ROOT_ABSOLUTE})
                endif()

                list (APPEND PLIBSYS_TEST_INCLUDE_DIRS ${BOOST_ROOT})
        endif()
endif()

list (APPEND PLIBSYS_TEST_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/../src ${CMAKE_BINARY_DIR})

if (MSVC)
        list (APPEND PLIBSYS_TEST_COMPILE_DEFS -D_CRT_SECURE_NO_WARNINGS)
endif()

macro (plibsys_add_test_executable TEST_NAME SRC_FILE)
        add_executable (${TEST_NAME} ${SRC_FILE})
        target_link_libraries (${TEST_NAME} ${Boost_LIBRARIES} plibsys)
        set_target_properties (${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

        # QNX requires libm for sqrt() and friends
        if (PLIBSYS_TESTS_TARGET_OS STREQUAL qnx)
                target_link_libraries (${TEST_NAME} m)
        endif()

        # Add include directories
        if (COMMAND target_include_directories)
                target_include_directories (${TEST_NAME} PUBLIC ${PLIBSYS_TEST_INCLUDE_DIRS})
        else()
                include_directories (${PLIBSYS_TEST_INCLUDE_DIRS})
        endif()

        # Add compile definitions
        if (COMMAND target_compile_definitions)
                target_compile_definitions (${TEST_NAME} PRIVATE ${PLIBSYS_TEST_COMPILE_DEFS})
        else()
                add_definitions (${PLIBSYS_TEST_COMPILE_DEFS})
        endif()

        if (${TEST_NAME} STREQUAL "plibraryloader_test")
                add_test (NAME ${TEST_NAME} COMMAND ${TEST_NAME} -- "$<TARGET_FILE:plibsys>")
        else()
                add_test (NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        endif()
endmacro()

plibsys_add_test_executable (patomic_test patomic_test.cpp)
plibsys_add_test_executable (pcondvariable_test pcondvariable_test.cpp)
plibsys_add_test_executable (pcryptohash_test pcryptohash_test.cpp)
plibsys_add_test_executable (perror_test perror_test.cpp)
plibsys_add_test_executable (pdir_test pdir_test.cpp)
plibsys_add_test_executable (pfile_test pfile_test.cpp)
plibsys_add_test_executable (phashtable_test phashtable_test.cpp)
plibsys_add_test_executable (pinifile_test pinifile_test.cpp)
plibsys_add_test_executable (plibraryloader_test plibraryloader_test.cpp)
plibsys_add_test_executable (plist_test plist_test.cpp)
plibsys_add_test_executable (pmacros_test pmacros_test.cpp)
plibsys_add_test_executable (pmain_test pmain_test.cpp)
plibsys_add_test_executable (pmem_test pmem_test.cpp)
plibsys_add_test_executable (pmutex_test pmutex_test.cpp)
plibsys_add_test_executable (pprocess_test pprocess_test.cpp)
plibsys_add_test_executable (prwlock_test prwlock_test.cpp)
plibsys_add_test_executable (psemaphore_test psemaphore_test.cpp)
plibsys_add_test_executable (pshmbuffer_test pshmbuffer_test.cpp)
plibsys_add_test_executable (pshm_test pshm_test.cpp)
plibsys_add_test_executable (psocket_test psocket_test.cpp)
plibsys_add_test_executable (psocketaddress_test psocketaddress_test.cpp)
plibsys_add_test_executable (pspinlock_test pspinlock_test.cpp)
plibsys_add_test_executable (pstring_test pstring_test.cpp)
plibsys_add_test_executable (ptimeprofiler_test ptimeprofiler_test.cpp)
plibsys_add_test_executable (ptree_test ptree_test.cpp)
plibsys_add_test_executable (ptypes_test ptypes_test.cpp)
plibsys_add_test_executable (puthread_test puthread_test.cpp)

# Print summary 
set (PLIBSYS_TESTS_SUMMARY
"
        == Tests configuration ==

        Build static tests:     ${PLIBSYS_TESTS_STATIC}
        Boost directory:        ${BOOST_ROOT}
")

message ("${PLIBSYS_TESTS_SUMMARY}")
