#include "Client.hpp"
#include "SDL.hpp"
#include "Server.hpp"
#include "Utils.hpp"

#include <spdlog/spdlog.h>

#include <thread>

using namespace std::chrono_literals;
using seconds_d = std::chrono::duration<double>;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    spdlog::set_level(spdlog::level::debug);

    // TODO: Configurable from CLI
    // TODO: Use a real CLI library, maybe Boost.ProgramOptions
    static constexpr std::chrono::milliseconds network_delay{250ms};
    static constexpr std::chrono::milliseconds server_tick_rate{500ms};
    static constexpr seconds_d client_update_interval{static_cast<double>(1) / 30};

    Client client;

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

    SDL_Rect rectangle{.x = initial_x, .y = y, .w = rect_width, .h = rect_height};

    SDL_Event event;
    bool left_key_pressed = false;
    bool right_key_pressed = false;

    auto last_frame_time = std::chrono::steady_clock::now();

    uint32_t sequence_number{0};

    // Game loop
    while (true) {
        client.process_server_messages();

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
            Client_message const msg{
              .duration = -frame_duration_s, .sequence_number = ++sequence_number};
            server.send(msg, network_delay);
            client.offset(update_position(client.offset(), -frame_duration_s.count())
            );
            client.save(msg);
        }
        else if (right_key_pressed) {
            Client_message const msg{
              .duration = frame_duration_s, .sequence_number = ++sequence_number};
            server.send(msg, network_delay);
            client.offset(update_position(client.offset(), frame_duration_s.count())
            );

            // TODO: Revisit all names of things
            client.save(msg);
        }

        RETURN_IF_SDL_ERROR(
          SDL_SetRenderDrawColor, renderer.get(), 255, 255, 255, 255
        );

        RETURN_IF_SDL_ERROR(SDL_RenderClear, renderer.get());

        // Get the current offset from the client
        const double offset = client.offset();

        // Offset the rectangle's position
        rectangle.x = static_cast<int>(std::round(initial_x + offset));

        // TODO: Change to circle
        RETURN_IF_SDL_ERROR(SDL_SetRenderDrawColor, renderer.get(), 0, 0, 255, 255);
        RETURN_IF_SDL_ERROR(SDL_RenderDrawRect, renderer.get(), &rectangle);

        // TODO: This could be renderer.present();
        SDL_RenderPresent(renderer.get());

        std::this_thread::sleep_for(client_update_interval);
    }
}
