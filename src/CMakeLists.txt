enable_compiler_warnings()

set(target_name netcode)

# WARNING: setting the WIN32 keyword here completely disables
# any console output: add_executable(netcode WIN32 ...)
#
# Lazy Foo tutorial uses -Wl,-subsystem,windows which is
# basically doing the same thing. So to see the printf's
# remove the WIN32 keyword.
add_executable(${target_name}
        main.cpp
        SDL.cpp
        Server.cpp
        Client.cpp
        Entity.hpp
)

target_link_libraries(${target_name}
        PRIVATE
            CLI11::CLI11
            ImGUI_SDL2
            SDL2::SDL2
            SDL2::SDL2main
            spdlog
)

# .dll has to be in the same directory as the .exe
if (WIN32)
    add_custom_command(
            TARGET ${target_name} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:${target_name}>"
            VERBATIM
    )
endif()
