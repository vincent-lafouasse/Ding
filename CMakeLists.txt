cmake_minimum_required(VERSION 3.16)

project(Ding VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(cmake/platform.cmake)
# include(cmake/fetch_gtest.cmake)

add_subdirectory(lib/juce)
add_subdirectory(Ding)
