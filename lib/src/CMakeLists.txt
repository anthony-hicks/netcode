include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

CPMAddPackage(
        NAME SDL2
        VERSION 2.28.4
        GITHUB_REPOSITORY "libsdl-org/SDL"
        GIT_TAG release-2.28.4
        SYSTEM TRUE
        EXCLUDE_FROM_ALL TRUE
)

enable_compiler_warnings()

add_library(SDL++
        sdl.cpp
)

target_include_directories(SDL++
        PUBLIC
        ${CMAKE_SOURCE_DIR}/lib/include
)

target_link_libraries(SDL++
        PUBLIC
        SDL2::SDL2
)
