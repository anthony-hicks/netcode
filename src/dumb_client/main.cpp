#include "Client.hpp"
#include "SDL.hpp"
#include "Server.hpp"

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <thread>

using namespace std::chrono_literals;
using seconds_d = std::chrono::duration<double>;

int main(int argc, char* argv[])
{
    CLI::App app;

    std::chrono::milliseconds network_delay{250ms};

    // Runescape server update rate
    // 1.67 updates per second or every 0.6s
    double server_update_rate_hz{1.67};

    // 30 updates per second or every 0.033s / 33ms
    double client_update_rate_hz{30};

    app.add_option("--delay", network_delay, "Network delay in ms between any client and the server");
    app.add_option("--tick-rate", server_update_rate_hz, "Server tick rate in hz");
    app.add_option("--client-rate", client_update_rate_hz, "Client update rate in hz");

    CLI11_PARSE(app, argc, argv);

    spdlog::set_level(spdlog::level::debug);

    const milliseconds_d server_update_interval{seconds_d{1.0 / server_update_rate_hz}};
    const milliseconds_d client_update_interval{seconds_d{1.0 / client_update_rate_hz}};

    Client client;
    Client spectator;

    Server server(network_delay);
    server.connect(&client);
    server.connect(&spectator);

    const std::jthread server_thread(
      [&server, &server_update_interval](const std::stop_token& stop_token) {
        while (!stop_token.stop_requested()) {
            server.update();
            std::this_thread::sleep_for(server_update_interval);
        }
      }
    );

    SDL::initialize(SDL_INIT_EVENTS);

    static constexpr int screen_height = 240;
    static constexpr int screen_width = 1450;

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
    SDL_Rect spectator_rect = rectangle;

    SDL_Event event;
    bool left_key_pressed = false;
    bool right_key_pressed = false;

    auto last_frame_time = std::chrono::steady_clock::now();

    // Game loop
    while (true) {
        client.process_server_messages();
        spectator.process_server_messages();

        // Compute the duration of the last frame, so we can determine
        // how far the player should move
        auto now = std::chrono::steady_clock::now();
        auto frame_duration_s = seconds_d{now - last_frame_time};
        last_frame_time = now;

        // Poll event queue. When the queue is empty, this function returns 0.
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                spdlog::info("[user] QUIT");
                return 0;
            }

            if (event.key.keysym.sym == SDLK_LEFT) {
                spdlog::info("[user] LEFT");
                left_key_pressed = event.type == SDL_KEYDOWN;
            }
            else if (event.key.keysym.sym == SDLK_RIGHT) {
                spdlog::info("[user] RIGHT");
                right_key_pressed = event.type == SDL_KEYDOWN;
            }
        }

        if (left_key_pressed) {
            Client_message const msg{.duration = -frame_duration_s};
            server.send(msg, network_delay);
        }
        else if (right_key_pressed) {
            Client_message const msg{.duration = frame_duration_s};
            server.send(msg, network_delay);
        }

        RETURN_IF_SDL_ERROR(SDL_SetRenderDrawColor, renderer.get(), 255, 255, 255, 255);
        RETURN_IF_SDL_ERROR(SDL_RenderClear, renderer.get());

        // Offset the rectangle's position
        rectangle.x = static_cast<int>(std::round(initial_x + client.offset()));

        // Offset the spectator view
        spectator_rect.x =
          static_cast<int>(std::round(initial_x + spectator.offset()));

        // Draw spectator view of p1
        RETURN_IF_SDL_ERROR(SDL_SetRenderDrawColor, renderer.get(), 255, 0, 0, 255);
        RETURN_IF_SDL_ERROR(SDL_RenderDrawRect, renderer.get(), &spectator_rect);

        // TODO: Change to circle
        RETURN_IF_SDL_ERROR(SDL_SetRenderDrawColor, renderer.get(), 0, 0, 255, 255);
        RETURN_IF_SDL_ERROR(SDL_RenderDrawRect, renderer.get(), &rectangle);

        // TODO: This could be renderer.present();
        SDL_RenderPresent(renderer.get());

        std::this_thread::sleep_for(client_update_interval);
    }
}
