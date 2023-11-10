#include "Server.hpp"

#include <asio/steady_timer.hpp>

#include <chrono>

using asio::ip::tcp;

/// Gotcha: must not call shared_from_this() in constructor, since
/// shared_from_this() requires there to already be a shared_ptr.
Server::Session::Session(asio::io_context* ctx, tcp::socket&& socket)
  : _ctx(ctx),
    _socket(std::move(socket)),
    _client_message{},
    _position_message{.position = 0},
    _tick_timer(*ctx)
{}

std::shared_ptr<Server::Session>
Server::Session::create(asio::io_context* ctx, tcp::socket&& socket)
{
    return std::shared_ptr<Session>(new Session(ctx, std::move(socket)));
}

void Server::Session::begin()
{
    async_tick();
    async_read();
}

void Server::Session::async_tick()
{
    auto self(shared_from_this());

    _tick_timer.expires_after(std::chrono::seconds(1));

    _tick_timer.async_wait([this, self](std::error_code ec) {
        if (ec) {
            spdlog::error("[server] timer failed");
            return;
        }

        asio::async_write(
          _socket,
          asio::buffer(&_position_message, _position_message.size()),
          [self](std::error_code error_code, std::size_t) {
              if (error_code) {
                  spdlog::error("[server] async_write: {}", error_code.message());
              }
          }
        );

        async_tick();
    });
}

void Server::Session::async_read()
{
    spdlog::debug("[server] submit async_read");

    auto self(shared_from_this());

    asio::async_read(
      _socket,
      asio::buffer(&_client_message, _client_message.size()),
      [this, self](std::error_code ec, std::size_t bytes_read) {
          if (ec == asio::error::eof) {
              spdlog::info(
                "[server] client disconnected: {}",
                _socket.local_endpoint().address().to_string()
              );
              return;
          }

          if (ec) {
              spdlog::error("[server] async_read: {}", ec.message());
              return;
          }

          spdlog::info(
            "[server] recv: {} ({}B)", _client_message.to_string(), bytes_read
          );

          if (_client_message.command == Command::move_left) {
              _position_message.position -= 1;
          }
          else if (_client_message.command == Command::move_right) {
              _position_message.position += 1;
          }
          else {
              spdlog::error("[server] invalid message");
              return;
          }

          spdlog::info("[server] update: position = {}", _position_message.position);

          // Continue reading from the client (restart the chain)
          async_read();
      }
    );
}

Server::Server(tcp::endpoint const& endpoint)
  : _acceptor(_ctx, endpoint)
{}

Server::~Server()
{
    stop();
}

void Server::start()
{
    spdlog::info(
      "[server] accepting connections on {}:{}",
      _acceptor.local_endpoint().address().to_string(),
      _acceptor.local_endpoint().port()
    );

    // Begin accepting connections
    async_accept();

    // BLOCKING: start the event loop
    _ctx.run();

    spdlog::info("[server] done");
}

void Server::stop()
{
    if (!_ctx.stopped()) {
        spdlog::info("[server] stopping...");
        _ctx.stop();
    }
}

void Server::async_accept()
{
    _acceptor.async_accept([this](std::error_code ec, tcp::socket socket) {
        // Continue to accept connections
        async_accept();

        if (ec) {
            spdlog::error("[server] async_accept: {}", ec.message());
            return;
        }

        spdlog::info(
          "[server] accepted new client connection: {}",
          socket.remote_endpoint().address().to_string()
        );

        Session::create(&_ctx, std::move(socket))->begin();
    });
}
