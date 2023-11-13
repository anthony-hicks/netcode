#include "Client.hpp"
#include "SDL.hpp"
#include "Server.hpp"

#include <spdlog/spdlog.h>

#include <iostream>
#include <string>
#include <thread>

using namespace std::chrono_literals;

// TODO: Change to updating speed/velocity on user input to calculate position

int main(int argc, char* argv[])
{
    const std::span args(argv, static_cast<unsigned long>(argc));

    if (args.size() != 3) {
        std::cerr << "usage: netcode <host> <port>\n";
        return 1;
    }

    // TODO: Use a real CLI library, maybe Boost.ProgramOptions
    std::string const host{args[1]};
    std::string const port{args[2]};

    spdlog::set_level(spdlog::level::debug);

    Client client;

    // TODO: Configurable from CLI
    static constexpr std::chrono::milliseconds network_delay{250ms};
    static constexpr std::chrono::milliseconds server_tick_rate{500ms};

    Server server(network_delay);
    server.connect(&client);

    const std::jthread server_thread([&server](const std::stop_token& stop_token) {
        while (!stop_token.stop_requested()) {
            server.update();
            std::this_thread::sleep_for(server_tick_rate);
        }
    });

    SDL::initialize(SDL_INIT_EVENTS);

    static constexpr int screen_height = 240;
    static constexpr int screen_width = 750;

    SDL::Window_ptr const window(SDL_CreateWindow(
      "Demo",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      screen_width,
      screen_height,
      SDL_WINDOW_SHOWN
    ));

    if (!window) {
        LOG_SDL_ERROR(SDL_CreateWindow, nullptr);
        return 1;
    }

    const SDL::Renderer_ptr renderer(
      SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED)
    );
    if (!renderer) {
        LOG_SDL_ERROR(SDL_CreateRenderer, nullptr);
        return 1;
    }

    constexpr int screen_center_x = screen_width / 2;
    constexpr int rect_height = 200;
    constexpr int rect_width = rect_height;
    constexpr int initial_x = screen_center_x - rect_width / 2;
    constexpr int y = (screen_height - rect_height) / 2;

    SDL_Rect rectangle{
      .x = initial_x,
      .y = y,
      .w = rect_width,
      .h = rect_height};

    SDL_Event event;

    // Game loop
    while (true) {
        client.process_server_messages();

        // Poll event queue. When the queue is empty, this function returns 0.
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                spdlog::info("[user] QUIT");
                return 0;
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT: {
                        spdlog::info("[user] LEFT");
                        server.send({.command = Command::move_left}, network_delay);
                    } break;
                    case SDLK_RIGHT: {
                        spdlog::info("[user] RIGHT");
                        server.send({.command = Command::move_right}, network_delay);
                    } break;
                }
            }
        }

        RETURN_IF_SDL_ERROR(SDL_SetRenderDrawColor, renderer.get(), 255, 255, 255, 255);
        RETURN_IF_SDL_ERROR(SDL_RenderClear, renderer.get());

        // Get the current offset from the client
        const int offset = client.offset();

        // Offset the rectangle's position
        rectangle.x = initial_x + offset;

        // TODO: Change to circle
        RETURN_IF_SDL_ERROR(SDL_SetRenderDrawColor, renderer.get(), 0, 0, 255, 255);
        RETURN_IF_SDL_ERROR(SDL_RenderDrawRect, renderer.get(), &rectangle);

        // TODO: This could be renderer.present();
        SDL_RenderPresent(renderer.get());
    }
}
