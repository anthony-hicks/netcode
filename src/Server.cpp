#include "Server.hpp"

using asio::ip::tcp;

/// Gotcha: must not call shared_from_this() in constructor, since
/// shared_from_this() requires there to already be a shared_ptr.
Server::Session::Session(tcp::socket&& socket)
  : _socket(std::move(socket))
{}

std::shared_ptr<Server::Session> Server::Session::create(tcp::socket&& socket)
{
    return std::shared_ptr<Session>(new Session(std::move(socket)));
}

void Server::Session::begin()
{
    static const std::string greeting = "Greetings, traveler";
    auto self(shared_from_this());
    asio::async_write(
      _socket,
      asio::buffer(greeting),
      [this, self](std::error_code ec, std::size_t) {
          if (ec) {
              spdlog::error("[server] async_write: {}", ec.message());
          }
      }
    );

    async_read();
}

void Server::Session::async_read()
{
    spdlog::debug("[server] submit async_read");

    auto self(shared_from_this());

    // NOTE: Optionally pass a length to asio::buffer
    _socket.async_read_some(
      asio::buffer(_message),
      [this, self](std::error_code ec, std::size_t bytes_read) {
          if (ec) {
              spdlog::error("[server] async_read: {}", ec.message());
              return;
          }

          spdlog::info(
            "[server] recv: {} ({}B)",
            std::string(_message.begin(), bytes_read),
            bytes_read
          );

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

        Session::create(std::move(socket))->begin();
    });
}
