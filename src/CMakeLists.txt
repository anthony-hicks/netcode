enable_compiler_warnings()

# WARNING: setting the WIN32 keyword here completely disables
# any console output: add_executable(netcode WIN32 ...)
#
# Lazy Foo tutorial uses -Wl,-subsystem,windows which is
# basically doing the same thing. So to see the printf's
# remove the WIN32 keyword.
add_executable(demo
        main.cpp
        SDL.cpp
        Server.cpp
        Client.cpp
)

add_executable(two
        two_in_one.cpp
        Client.cpp
        Server.cpp
)
target_link_libraries(two asio spdlog)

target_link_libraries(demo
        PRIVATE
            SDL2::SDL2
            SDL2::SDL2main
            spdlog
            asio
            Microsoft.GSL::GSL
)

# .dll has to be in the same directory as the .exe
if (WIN32)
    add_custom_command(
            TARGET demo POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:demo>"
            VERBATIM
    )
endif()
