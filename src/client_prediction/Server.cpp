#include "Server.hpp"

#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

Server::Server(std::chrono::milliseconds network_delay)
  : _network_delay(network_delay)
{}

void Server::connect(Client* client)
{
    _clients.push_back(client);
}

void Server::send(const Command_message& cmd, std::chrono::milliseconds delay)
{
    const Message msg{
      .message = cmd, .recv_timestamp = std::chrono::system_clock::now() + delay};

    {
        const std::scoped_lock lock(_queue_mutex);
        _queue.push_back(msg);
    }
}

void Server::update()
{
    // Process client messages
    {
        const std::scoped_lock lock(_queue_mutex);

        for (const auto& msg : _queue) {
            if (msg.recv_timestamp <= std::chrono::system_clock::now()) {
                spdlog::info("[server] recv: {}", to_string(msg.message.command));

                switch (msg.message.command) {
                    case Command::move_left:
                        _state.position -= 1;
                        break;
                    case Command::move_right:
                        _state.position += 1;
                        break;
                }

                spdlog::info("[server] update: position = {}", _state.position);
            }
        }

        std::erase_if(_queue, [](const auto& msg) {
            return msg.recv_timestamp <= std::chrono::system_clock::now();
        });
    }

    // Send clients game state
    for (auto& client : _clients) {
        client->send(_state, _network_delay);
    }
}