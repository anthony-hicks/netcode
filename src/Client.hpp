#pragma once

#include "common.hpp"
#include "Message.hpp"

#include <asio.hpp>
#include <spdlog/spdlog.h>

#include <queue>
#include <string>

/// @brief TODO
class Client {
    asio::io_context* _ctx;
    asio::ip::tcp::socket _socket;
    Position_Message _position;
    std::queue<Command_Message> _write_queue;

public:
    /// @brief Create a client and attempt to [async] connect, which starts the chain
    /// of communication.
    Client(asio::io_context* ctx, std::string_view host, std::string_view port);

    /// Closes connection with server if not already closed
    ~Client();

    DISABLE_CTOR(Client);
    DISABLE_COPY(Client);
    DISABLE_MOVE(Client);

    /// Explicitly close the connection
    void close();

    /// Write (enqueue) a message to be sent to the server
    void async_write(const Command_Message& message);

private:
    void async_connect(asio::ip::tcp::resolver::results_type const& endpoints);

    void async_read();
};
