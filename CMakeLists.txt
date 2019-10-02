set (CMAKE_CXX_STANDARD 11)
cmake_minimum_required(VERSION 3.5)

project(maeparser)

option(MAEPARSER_RIGOROUS_BUILD "Abort the build if the compiler issues any warnings" OFF )

if(MSVC)
    # C4251 disables warnings for export STL containers as arguments (returning a vector of things)
    add_definitions(/wd4251 /wd4275 /wd4996 /D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

if(MAEPARSER_RIGOROUS_BUILD)
if(MSVC)
    add_definitions(/WX)
else(MSVC)
    add_definitions(-Wall -Wextra -Werror)
endif(MSVC)
endif(MAEPARSER_RIGOROUS_BUILD)

find_package(Boost COMPONENTS filesystem iostreams unit_test_framework REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_library(maeparser SHARED Buffer.cpp MaeBlock.cpp MaeParser.cpp Reader.cpp Writer.cpp)
SET_TARGET_PROPERTIES (maeparser
    PROPERTIES
       VERSION 1.2.2
       SOVERSION 1
)
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

set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --tool=memcheck \
        --time-stamp=yes --num-callers=20 --gen-suppressions=all --leak-check=full \
        --show-reachable=yes --trace-children=yes --error-exitcode=29")

include(CTest)
add_subdirectory(test)

install(FILES Buffer.hpp MaeBlock.hpp  MaeParserConfig.hpp  MaeParser.hpp  Reader.hpp MaeConstants.hpp
        DESTINATION include/maeparser)

