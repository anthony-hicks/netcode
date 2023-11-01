#include "Client.hpp"
#include "Server.hpp"

#include <asio.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

using asio::ip::tcp;
using namespace std::chrono_literals;

int main(int argc, char* argv[])
{
    //    spdlog::set_level(spdlog::level::trace);

    if (argc != 3) {
        std::cerr << "usage: two <host> <port>\n";
        return 1;
    }

    tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[2]));

    Server server(endpoint);

    std::jthread const server_thread([&server] {
        server.start();
    });

    asio::io_context client_context;
    Client client(&client_context, "localhost", argv[2]);

    std::jthread client_thread([&client_context]() {
        client_context.run();
    });

    // TODO: Why do we have to sleep for a bit to prime the context?
    // TODO: Would using io::work fix it?
    std::this_thread::sleep_for(0.5s);

    client.async_write("Hi, hello.");

    // TODO: Wait until...ctrl+c?
    std::this_thread::sleep_for(1s);

    client.async_write("Goodbye!");
    std::this_thread::sleep_for(0.5s);
    //     client.close();
    client_context.stop();

    server.stop();

    return 0;
}
