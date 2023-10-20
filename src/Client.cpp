#include "Client.hpp"

#include <asio.hpp>
#include <spdlog/spdlog.h>

using asio::ip::tcp;

namespace app {

void async_read_foo()
{
    std::string message;
    asio::async_read(
      socket,
      asio::buffer(message),
      [](asio::error_code error_code, std::size_t length) {
          if (error_code) {
              spdlog::error("[client] error TODO what can trigger this");
              return;
          }
      }
    )
}

// TODO (client)
//  - async reads (callback) for server updates
//  - async write to server (write can happen on main thread I think)
void foo(asio::io_context& io_context)
{
    tcp::resolver resolver(io_context);
    // TODO: params
    auto endpoints = resolver.resolve(tcp::v4(), "localhost", std::to_string(50000));
    tcp::socket socket(io_context);
    asio::async_connect(
      socket,
      endpoints,
      [](asio::error_code error_code, tcp::endpoint endpoint) {
          if (error_code) {
              spdlog::error("[client] async_connect");
              return;
          }

          spdlog::info("[client] connected to {}", endpoint.address().to_string());
          async_read_foo();
      }
    );
}
}
