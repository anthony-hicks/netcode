#include "Client.hpp"
#include "Message.hpp"
#include "SDL.hpp"
#include "Server.hpp"

#include <asio.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

using asio::ip::tcp;
using namespace std::chrono_literals;

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "usage: netcode <host> <port>\n";
        return 1;
    }

    // TODO: Use a real CLI library, maybe Boost.ProgramOptions
    std::string const host{argv[1]};
    std::string const port{argv[2]};

    spdlog::set_level(spdlog::level::debug);

    // TODO: Consider whether the network connection should launch before or after
    //  graphics

    // Create the server on its own thread. Normally the server would be its own
    // process, but to simplify testing a little bit, we're going to spawn it as
    // a thread. We will still communicate with it over the network, however.
    Server server(tcp::endpoint(tcp::v4(), std::stoul(port)));
    std::jthread const server_thread([&server] {
        server.start();
    });

    asio::io_context ctx;
    Client client(&ctx, host, port);

    // Run the client's session/event loop with the server on its own thread. This
    // thread asynchronously communicates with the server.
    std::jthread const client_thread([&ctx] {
        ctx.run();
    });

    // Sleep for a very short amount of time so the event loops are ready
    std::this_thread::sleep_for(10ms);

    SDL::initialize(SDL_INIT_EVENTS);

    static constexpr int screen_height = 640;
    static constexpr int screen_width = 480;

    SDL::Window_ptr const window(SDL_CreateWindow(
      "Demo",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      screen_width,
      screen_height,
      SDL_WINDOW_SHOWN
    ));

    if (!window) {
        return 1;
    }

    SDL_Surface* screen_surface = SDL::GetWindowSurface(window.get());
    if (screen_surface == nullptr) {
        return 1;
    }

    SDL_Event event;

    // Game loop
    while (true) {
        // Poll event queue. When the queue is empty, this function returns 0.
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                spdlog::info("[user] QUIT");

                // TODO: Graceful termination without explicit stops here
                ctx.stop();
                server.stop();
                return 0;
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT: {
                        spdlog::info("[user] LEFT");
                        client.async_write({.command = Command::move_left});
                    } break;
                    case SDLK_RIGHT: {
                        spdlog::info("[user] RIGHT");
                        client.async_write({.command = Command::move_right});
                    } break;
                }
            }
        }

        if (!SDL::UpdateWindowSurface(window.get())) {
            return 1;
        }
    }
}
