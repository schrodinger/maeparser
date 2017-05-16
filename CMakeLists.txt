set (CMAKE_CXX_STANDARD 11)
cmake_minimum_required(VERSION 3.2)

project(maeparser)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

add_library(maeparser Buffer.cpp MaeBlock.cpp MaeParser.cpp Reader.cpp)

enable_testing()
add_subdirectory(test)

add_test(NAME unittest COMMAND test/unittest)

