project (tests CXX)
set (OUTPUT_DIR ${CMAKE_BINARY_DIR})

find_package (Boost COMPONENTS unit_test_framework REQUIRED)

include_directories (${Boost_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/../src ${CMAKE_BINARY_DIR})

set (PSTRING_TEST_SRCS pstring_test.cpp)
add_executable (pstring_test ${PSTRING_TEST_SRCS})
target_link_libraries (pstring_test ${Boost_LIBRARIES} plib)
set_target_properties (pstring_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

set (PLIST_TEST_SRCS plist_test.cpp)
add_executable (plist_test ${PLIST_TEST_SRCS})
target_link_libraries (plist_test ${Boost_LIBRARIES} plib)
set_target_properties (plist_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

add_test (NAME pstring_test COMMAND pstring_test)
add_test (NAME plist_test COMMAND plist_test)