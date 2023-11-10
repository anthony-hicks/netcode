#include "Client.hpp"
#include "Message.hpp"
#include "SDL.hpp"
#include "Server.hpp"

#include <asio.hpp>
#include <spdlog/spdlog.h>

#include <iostream>
#include <string>
#include <thread>

using asio::ip::tcp;
using namespace std::chrono_literals;

class AsyncServer {
    Server _server;
    std::jthread _thread;

public:
    explicit AsyncServer(const tcp::endpoint& endpoint)
      : _server(endpoint),
        _thread([this] {
            _server.start();
        })
    {}

    ~AsyncServer() { _server.stop(); }
};

class AsyncClient : public Client {
    asio::io_context* _ctx;
    std::jthread _thread;

public:
    explicit AsyncClient(
      asio::io_context* ctx, std::string_view host, std::string_view port
    )
      : Client(ctx, host, port),
        _ctx(ctx),
        _thread([this] {
            _ctx->run();
        })
    {}

    ~AsyncClient() { _ctx->stop(); }
};

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

    // Create the server on its own thread. Normally the server would be its own
    // process, but to simplify testing a little bit, we're going to spawn it as
    // a thread. We will still communicate with it over the network, however.
    AsyncServer const async_server(
      tcp::endpoint(tcp::v4(), static_cast<unsigned short>(std::stoul(port)))
    );

    // Run the client's session/event loop with the server on its own thread. This
    // thread asynchronously communicates with the server.
    asio::io_context client_ctx;
    AsyncClient client(&client_ctx, host, port);

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
