include_directories(..)

add_executable(unittest
    MainTestSuite.cpp BufferTest.cpp MaeBlockTest.cpp MaeParserTest.cpp ReaderTest.cpp UsageDemo.cpp)

target_link_libraries(unittest maeparser Boost::unit_test_framework)

if(MSVC)
    add_custom_command(TARGET unittest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_BINARY_DIR}/\$\(Configuration\)/maeparser.dll" "${CMAKE_BINARY_DIR}/test/\$\(Configuration\)/maeparser.dll")
endif(MSVC)

