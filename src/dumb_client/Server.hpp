#pragma once

#include "common.hpp"
#include "Message.hpp"

#include <asio.hpp>
#include <spdlog/spdlog.h>

// TODO: When can asio throw?
/// @brief TODO
class Server {
    using tcp = asio::ip::tcp;

    /// @brief Client session
    ///
    /// Session implements the actual communication logic, and
    /// uses the shared_from_this() pattern to manage its own
    /// lifetime.
    class Session : public std::enable_shared_from_this<Session> {
        asio::io_context* _ctx;
        tcp::socket _socket;
        Command_Message _client_message;
        Position_Message _position_message;

        asio::steady_timer _tick_timer;
        std::chrono::milliseconds _tick_interval;

        /// @brief Private constructor so you can't accidentally get a
        /// std::bad_weak_ptr exception thrown when calling shared_from_this().
        explicit Session(
          asio::io_context* ctx,
          tcp::socket&& socket,
          std::chrono::milliseconds tick_interval
        );

    public:
        /// Create the session. You MUST call .begin() to begin the session
        /// with the client (asynchronous).
        [[nodiscard]]
        static std::shared_ptr<Session> create(
          asio::io_context* ctx,
          tcp::socket&& socket,
          std::chrono::milliseconds tick_interval
        );

        Session() = delete;
        ~Session() = default;
        DISABLE_COPY(Session);
        DISABLE_MOVE(Session);

        /// (asynchronously) Begin the session
        void begin();

    private:
        void async_tick();
        void async_read();
    };

public:
    /// Create the server. Must call .start() to start accepting connections.
    explicit Server(
      tcp::endpoint const& endpoint, std::chrono::milliseconds tick_interval
    );

    /// Calls stop().
    ~Server();

    /// [BLOCKING] Begin accepting connections.
    void start();

    /// Stops the server if it hasn't been already.
    void stop();

private:
    void async_accept();

    asio::io_context _ctx;
    tcp::acceptor _acceptor;
    tcp::endpoint _endpoint;
    std::chrono::milliseconds _tick_interval;
};
