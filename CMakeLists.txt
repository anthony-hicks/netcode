cmake_minimum_required(VERSION 3.21)
project(netcode VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)
CPMAddPackage(
        NAME SDL2
        VERSION 2.28.4
        GITHUB_REPOSITORY "libsdl-org/SDL"
        GIT_TAG release-2.28.4
        SYSTEM TRUE
        EXCLUDE_FROM_ALL TRUE
)

CPMAddPackage("gh:CLIUtils/CLI11@2.3.2")
CPMAddPackage("gh:gabime/spdlog@1.12.0")
find_package(Threads REQUIRED)
CPMAddPackage("gh:chriskohlhoff/asio#asio-1-28-0@1.28.0")
if (asio_ADDED)
        add_library(asio INTERFACE)
        target_include_directories(asio SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)
        target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
        target_link_libraries(asio INTERFACE Threads::Threads)
endif ()

include(cmake/compiler_warnings.cmake)
add_subdirectory(src)
