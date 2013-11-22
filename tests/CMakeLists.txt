project (tests CXX)
set (OUTPUT_DIR ${CMAKE_BINARY_DIR})

find_package (Boost COMPONENTS unit_test_framework REQUIRED)

include_directories (${Boost_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/../src ${CMAKE_BINARY_DIR})

set (PLIST_TEST_SRCS plist_test.cpp)
add_executable (plist_test ${PLIST_TEST_SRCS})
target_link_libraries (plist_test ${Boost_LIBRARIES} plib)
set_target_properties (plist_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

set (PHASH_TEST_SRCS phash_test.cpp)
add_executable (phash_test ${PHASH_TEST_SRCS})
target_link_libraries (phash_test ${Boost_LIBRARIES} plib)
set_target_properties (phash_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

set (PSEMAPHORE_TEST_SRCS psemaphore_test.cpp)
add_executable (psemaphore_test ${PSEMAPHORE_TEST_SRCS})
target_link_libraries (psemaphore_test ${Boost_LIBRARIES} plib)
set_target_properties (psemaphore_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

set (PSHM_TEST_SRCS pshm_test.cpp)
add_executable (pshm_test ${PSHM_TEST_SRCS})
target_link_libraries (pshm_test ${Boost_LIBRARIES} plib)
set_target_properties (pshm_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

set (PSTRING_TEST_SRCS pstring_test.cpp)
add_executable (pstring_test ${PSTRING_TEST_SRCS})
target_link_libraries (pstring_test ${Boost_LIBRARIES} plib)
set_target_properties (pstring_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

add_test (NAME plist_test COMMAND plist_test)
add_test (NAME phash_test COMMAND phash_test)
add_test (NAME psemaphore_test COMMAND psemaphore_test)
add_test (NAME pshm_test COMMAND pshm_test)
add_test (NAME pstring_test COMMAND pstring_test)