#include "Client.hpp"
#include "Config.hpp"
#include "SDL.hpp"
#include "Server.hpp"
#include "Utils.hpp"

#include <CLI/CLI.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <spdlog/spdlog.h>

#include <thread>

using namespace std::chrono_literals;

int main(int argc, char* argv[])
{
    CLI::App app;

    Config config{};

    std::size_t interpolation_tick_delay{1};

    app.add_option(
      "--interp-delay",
      interpolation_tick_delay,
      "Number of ticks to delay entities for interpolation"
    );

    CLI11_PARSE(app, argc, argv);

    spdlog::set_level(spdlog::level::debug);

    Client client;
    Client spectator;

    Server server(config.latency());
    client.entity_id(server.connect(&client));
    spectator.entity_id(server.connect(&spectator));

    const std::jthread server_thread([&server,
                                      &config](const std::stop_token& stop_token) {
        while (!stop_token.stop_requested()) {
            server.update();
            std::this_thread::sleep_for(config.server_update_interval());
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& gui_io = ImGui::GetIO();
    (void)gui_io;

    // Setup GUI style
    ImGui::StyleColorsDark();

    // Set up Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window.get(), renderer.get());
    ImGui_ImplSDLRenderer2_Init(renderer.get());

    constexpr int screen_center_x = screen_width / 2;
    constexpr int rect_height = 200;
    constexpr int rect_width = rect_height;
    constexpr int initial_x = screen_center_x - rect_width / 2;
    constexpr int y = (screen_height - rect_height) / 2;

    SDL_Rect rectangle{.x = initial_x, .y = y, .w = rect_width, .h = rect_height};
    SDL_Rect spectator_rect = rectangle;

    SDL_Event event;
    bool left_key_pressed = false;
    bool right_key_pressed = false;

    auto last_frame_time = std::chrono::steady_clock::now();

    uint32_t sequence_number{0};

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
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window.get()))) {
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
              .entity_id = client.entity_id(),
              .duration = -frame_duration_s,
              .sequence_number = ++sequence_number};
            server.send(msg, config.latency());

            // Client prediction
            if (config.prediction()) {
                client.offset(
                  update_position(client.offset(), -frame_duration_s.count())
                );
            }

            // Reconciliation
            if (config.reconciliation()) {
                client.save(msg);
            }
        }
        else if (right_key_pressed) {
            Client_message const msg{
              .entity_id = client.entity_id(),
              .duration = frame_duration_s,
              .sequence_number = ++sequence_number};
            server.send(msg, config.latency());

            // Client prediction
            if (config.prediction()) {
                client.offset(
                  update_position(client.offset(), frame_duration_s.count())
                );
            }

            // TODO: Revisit all names of things
            // TODO: Finish docs
            if (config.reconciliation()) {
                // [reconciliation] Save the
                client.save(msg);
            }
        }

        if (config.interpolation()) {
            // NOTE: Normally all clients would interpolate, but since we only have
            //  one entity in our world, then only the spectator needs to
            //  interpolate.
            spectator.interpolate_entities(
              config.server_update_interval(), interpolation_tick_delay);
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Configuration");

        ImGui::Checkbox("Prediction", &config.prediction());
        ImGui::Checkbox("Reconciliation", &config.reconciliation());
        ImGui::Checkbox("Interpolation", &config.interpolation());

        static int latency_ms = static_cast<int>(config.latency().count());
        if (ImGui::SliderInt("Lag (ms)", &latency_ms, 0, 1000)) {
            config.latency(std::chrono::milliseconds{latency_ms});
            server.set_network_delay(config.latency());
        }

        static float server_hz = config.server_update_rate();
        if (ImGui::SliderFloat("Server (hz)", &server_hz, 0.1F, 250.0F)) {
            config.server_update_rate(server_hz);
        }

        static float client_hz = config.client_update_rate();
        if (ImGui::SliderFloat("Client (hz)", &client_hz, 1.0F, 250.0F)) {
            config.client_update_rate(client_hz);
        }

        if (ImGui::Button("Reset")) {
            config = Config();

            latency_ms = static_cast<int>(config.latency().count());
            server.set_network_delay(config.latency());

            client_hz = config.client_update_rate();
            server_hz = config.server_update_rate();
        }

        ImGui::Text(
          "%.3f ms/frame (%.1f FPS)",
          1000.0 / static_cast<double>(ImGui::GetIO().Framerate),
          static_cast<double>(ImGui::GetIO().Framerate)
        );

        ImGui::End();
        ImGui::Render();

        RETURN_IF_SDL_ERROR(
          SDL_SetRenderDrawColor, renderer.get(), 255, 255, 255, 255
        );

        RETURN_IF_SDL_ERROR(SDL_RenderClear, renderer.get());

        // TODO: Not sure why this is after those renderer calls
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

        // Offset the rectangle's position
        rectangle.x = static_cast<int>(std::round(initial_x + client.offset()));

        // Offset the spectator view
        spectator_rect.x =
          static_cast<int>(std::round(initial_x + spectator.offset()));

        // TODO: Change to circle
        RETURN_IF_SDL_ERROR(SDL_SetRenderDrawColor, renderer.get(), 0, 0, 255, 255);
        RETURN_IF_SDL_ERROR(SDL_RenderDrawRect, renderer.get(), &rectangle);

        // Draw spectator view of p1
        RETURN_IF_SDL_ERROR(SDL_SetRenderDrawColor, renderer.get(), 255, 0, 0, 255);
        RETURN_IF_SDL_ERROR(SDL_RenderDrawRect, renderer.get(), &spectator_rect);

        // TODO: This could be renderer.present();
        SDL_RenderPresent(renderer.get());

        std::this_thread::sleep_for(config.client_update_interval());
    }

    // Cleanup (TODO: RAII, unreachable)
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
