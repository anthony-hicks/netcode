#include "Server.hpp"

#include "Utils.hpp"

#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

Server::Server(std::chrono::milliseconds network_delay)
  : _network_delay(network_delay)
{}

std::size_t Server::connect(Client* client)
{
    const size_t entity_id{_clients.size()};

    _clients.push_back(client);

    const Entity_state state{.position = 0.0, .id = entity_id};
    _states.push_back(state);
    _last_processed_inputs.push_back(0);

    return entity_id;
}

void Server::send(const Client_message& cmd, std::chrono::milliseconds delay)
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

            // Only process messages that get past the network delay
            if (msg.recv_timestamp <= std::chrono::system_clock::now()) {
                spdlog::info("[server] recv: (seq={}, duration={:.3f})", msg.message.sequence_number, msg.message.duration.count());

                auto id = msg.message.entity_id;
                _states[id].position = update_position(_states[id].position, msg.message.duration.count());
                _last_processed_inputs[id] = msg.message.sequence_number;

                spdlog::info("[server] update: position = {:.3f}", _states[id].position);
            }
        }

        std::erase_if(_queue, [](const auto& msg) {
            return msg.recv_timestamp <= std::chrono::system_clock::now();
        });
    }

    // Send clients game state
    for (auto& client : _clients) {

        Server_update update_msg;
        update_msg.states = _states;

        // Only send the last input processed for this client, it doesn't care
        // about the other clients
        update_msg.last_processed_input = _last_processed_inputs[client->entity_id()];

        client->send(update_msg, _network_delay);
    }
}
