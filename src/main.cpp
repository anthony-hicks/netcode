#include "SDL.hpp"

#include <asio.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <ctime>
#include <string>
#include <thread>

using asio::ip::tcp;
using namespace std::chrono_literals;

std::string make_daytime_string()
{
    using namespace std;  // For time_t, time and ctime;
    std::time_t now = std::time(0);
    return std::ctime(&now);
}

int main(int argc, char* argv[])
{
    spdlog::set_level(spdlog::level::debug);

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

    // TODO: start server
    //  - may want to follow the paradigm of: add asio::work to context, where
    //  the context is .run() on a background thread, and keep adding callbacks
    //  when we have successive data to continue to read.

    SDL_Event event;

    // Game loop
    while (true) {
        // Poll event queue. When the queue is empty, this function returns 0.
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                spdlog::info("[user] QUIT");
                return 0;
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        spdlog::info("[user] LEFT");
                        break;
                    case SDLK_RIGHT:
                        spdlog::info("[user] RIGHT");
                        break;
                }
            }
        }

        if (!SDL::UpdateWindowSurface(window.get())) {
            return 1;
        }
    }

    return 0;
}
