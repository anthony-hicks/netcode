#include "Server.hpp"

#include <asio.hpp>
#include <spdlog/spdlog.h>

using asio::ip::tcp;

namespace app {
// TODO (server)
//  -
//  - async reads (callback) for client updates
//  - async write to client, but do it periodically (server update rate)
void foo(asio::io_context& io_context, short port)
{
    // TODO: params
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    tcp::socket socket(io_context);

    acceptor.async_accept(socket, [&socket](auto error_code) {
        if (error_code) {
            spdlog::error("[server] async_accept: {}", error_code.message());
            // TODO: throw? not sure why the example keeps on going
        }
        else {
            std::string message;
            socket.async_read_some(asio::buffer(message), [](auto error_code, auto) {
                if (error_code) {
                    spdlog::error(
                      "[server] async_read_some: {}", error_code.message()
                    );
                    return;
                }

                // TODO: Do something with client data
                spdlog::info("[server] (recv)");
            });

            // do_accept() again?
        }
    });
}
}
