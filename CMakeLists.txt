set (CMAKE_CXX_STANDARD 11)
cmake_minimum_required(VERSION 3.2)

project(maeparser)

if(MSVC)
    # C4251 disables warnings for export STL containers as arguments (returning a vector of things)
    add_definitions(/wd4251 /D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

find_package(Boost COMPONENTS iostreams unit_test_framework REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_library(maeparser SHARED Buffer.cpp MaeBlock.cpp MaeParser.cpp Reader.cpp)
target_link_libraries(maeparser Boost::disable_autolinking Boost::dynamic_linking Boost::iostreams)

target_include_directories(maeparser PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)

install(TARGETS maeparser
    EXPORT maeparser-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

INSTALL(EXPORT maeparser-targets
    FILE ${PROJECT_NAME}-config.cmake
    DESTINATION lib/cmake)

target_compile_definitions(maeparser PRIVATE IN_MAEPARSER)

set_property(TARGET maeparser PROPERTY CXX_VISIBILITY_PRESET "hidden")

enable_testing()
add_subdirectory(test)

add_test(NAME unittest COMMAND ${CMAKE_BINARY_DIR}/test/unittest
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test)

install(FILES Buffer.hpp MaeBlock.hpp  MaeParserConfig.hpp  MaeParser.hpp  Reader.hpp MaeConstants.hpp
        DESTINATION include/maeparser)

