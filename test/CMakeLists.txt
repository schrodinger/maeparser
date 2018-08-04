find_package(Boost COMPONENTS unit_test_framework system iostreams REQUIRED)
find_package( ZLIB REQUIRED )

include_directories(..)
include_directories( ${ZLIB_INCLUDE_DIRS} )

add_executable(unittest MainTestSuite.cpp BufferTest.cpp MaeBlockTest.cpp MaeParserTest.cpp ReaderTest.cpp UsageDemo.cpp)

if(MSVC)
add_custom_command(TARGET unittest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different  "${PROJECT_BINARY_DIR}/\$\(Configuration\)/maeparser.dll" "${CMAKE_BINARY_DIR}/test/\$\(Configuration\)/maeparser.dll")
endif(MSVC)

target_link_libraries(unittest
    maeparser
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
