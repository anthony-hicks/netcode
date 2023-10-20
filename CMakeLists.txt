cmake_minimum_required(VERSION 3.21)
project(netcode VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/compiler_warnings.cmake)

add_subdirectory(lib/src)
add_subdirectory(src)
