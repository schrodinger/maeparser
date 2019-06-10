include_directories(..)

add_executable(unittest
    MainTestSuite.cpp BufferTest.cpp MaeBlockTest.cpp MaeParserTest.cpp ReaderTest.cpp WriterTest.cpp UsageDemo.cpp)

target_link_libraries(unittest maeparser Boost::unit_test_framework Boost::filesystem)

get_filename_component(TEST_SAMPLES_PATH ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
target_compile_definitions(unittest PRIVATE "TEST_SAMPLES_PATH=\"${TEST_SAMPLES_PATH}\"")

add_test(NAME unittest COMMAND ${CMAKE_BINARY_DIR}/test/unittest
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)

if(MSVC)
    add_custom_command(TARGET unittest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_BINARY_DIR}/\$\(Configuration\)/maeparser.dll" "${CMAKE_BINARY_DIR}/test/\$\(Configuration\)/maeparser.dll")
endif(MSVC)

