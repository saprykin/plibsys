project (tests CXX)
set (OUTPUT_DIR ${CMAKE_BINARY_DIR})

find_package (Boost COMPONENTS unit_test_framework REQUIRED)

set (PSTRING_TEST_SRCS pstring_test.cpp)

include_directories (${Boost_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/../src ${CMAKE_BINARY_DIR})
add_executable (pstring_test ${PSTRING_TEST_SRCS})
target_link_libraries (pstring_test ${Boost_LIBRARIES} plib)
set_target_properties (pstring_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

add_test (NAME pstring_test COMMAND pstring_test)