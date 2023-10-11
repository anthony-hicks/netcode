cmake_minimum_required(VERSION 3.26)

project(netcode VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME SDL2
    VERSION 2.28.4
    GITHUB_REPOSITORY "libsdl-org/SDL"
    GIT_TAG release-2.28.4
    SYSTEM TRUE
    EXCLUDE_FROM_ALL TRUE
)

include(cmake/compiler_warnings.cmake)
enable_compiler_warnings()

# WARNING: setting the WIN32 keyword here completely disables
# any console output: add_executable(netcode WIN32 ...)
#
# Lazy Foo tutorial uses -Wl,-subsystem,windows which is
# basically doing the same thing. So to see the printf's
# remove the WIN32 keyword.
add_executable(netcode
    main.cpp
)

target_link_libraries(netcode
    PUBLIC
        SDL2::SDL2main
        SDL2::SDL2
)

# .dll has to be in the same directory as the .exe
if (WIN32)
    add_custom_command(
        TARGET netcode POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:netcode>"
        VERBATIM
    )
endif()
