#include "Client.hpp"

#include <asio.hpp>
#include <spdlog/spdlog.h>

using asio::ip::tcp;

Client::Client(asio::io_context* ctx, std::string_view host, std::string_view port)
  : _ctx(ctx),
    _socket(*ctx),
    _position({.position = 0})
{
    tcp::resolver resolver(*_ctx);
    auto endpoints = resolver.resolve(host, port);
    async_connect(endpoints);
}

Client::~Client()
{
    close();
}

void Client::close()
{
    asio::post(*_ctx, [this] {
        if (_socket.is_open()) {
            _socket.close();
        }
    });
}

void Client::async_write(const Command_Message& message)
{
    asio::post(*_ctx, [this, message]() {
        _write_queue.push(message);

        asio::async_write(
          _socket,
          asio::buffer(&_write_queue.front(), _write_queue.front().size()),
          [this](std::error_code ec, std::size_t bytes_written) {
              if (ec) {
                  spdlog::error("[client] async_write: {}", ec.message());
                  // TODO: _socket.close()?
                  _socket.close();
                  return;
              }

              spdlog::info(
                "[client] send: {} ({}B)",
                _write_queue.front().to_string(),
                bytes_written
              );

              _write_queue.pop();

              // TODO:
              // NOTE: chat_client.cpp (from ASIO examples) has a call to
              // do_write() here if there are more messages on the queue to
              // write. But from what I understand, you can only get a message
              // on the queue by calling write(), which itself posts a call to
              // async_write. So there would already be a 1:1 without do_write
              // needing to make more async_write calls to "drain" the queue.
          }
        );
    });
}

void Client::async_connect(tcp::resolver::results_type const& endpoints)
{
    asio::async_connect(
      _socket,
      endpoints,
      [this](std::error_code ec, tcp::endpoint const& endpoint) {
          if (ec) {
              spdlog::error("[client] async_connect: {}", ec.message());
              return;
          }

          spdlog::info(
            "[client] connected to server {}:{}",
            endpoint.address().to_string(),
            endpoint.port()
          );

          async_read();
      }
    );
}

void Client::async_read()
{
    asio::async_read(
      _socket,
      asio::buffer(&_position, _position.load().size()),
      [this](std::error_code ec, std::size_t bytes_read) {
          if (ec) {
              spdlog::error("[client] async_read: {}", ec.message());
              _socket.close();
              // TODO: socket.close()?
              return;
          }

          spdlog::info("[client] recv: {} ({}B)", _position.load().position, bytes_read);

          async_read();
      }
    );
}
