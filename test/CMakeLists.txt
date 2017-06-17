find_package(Boost COMPONENTS unit_test_framework REQUIRED)

include_directories(..)

add_executable(unittest MainTestSuite.cpp BufferTest.cpp MaeBlockTest.cpp MaeParserTest.cpp ReaderTest.cpp UsageDemo.cpp)

target_link_libraries(unittest
    maeparser
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
