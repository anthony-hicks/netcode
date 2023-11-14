#include "Client.hpp"

#include <spdlog/spdlog.h>

void Client::send(const State_message& state, std::chrono::milliseconds delay)
{
    const Message msg{
      .message = state, .recv_timestamp = std::chrono::system_clock::now() + delay};

    {
        const std::scoped_lock lock(_queue_mutex);
        _queue.push_back(msg);
    }
}

void Client::process_server_messages()
{
    const std::scoped_lock lock(_queue_mutex);

    for (const auto& msg : _queue) {
        if (msg.recv_timestamp <= std::chrono::system_clock::now()) {
            _offset = msg.message.position;

            spdlog::info("[client] recv: {}", msg.message.position);
        }
    }

    std::erase_if(_queue, [](const auto& msg) {
        return msg.recv_timestamp <= std::chrono::system_clock::now();
    });
}

double Client::offset() const
{
    return _offset;
}

void Client::offset(double offset)
{
    _offset = offset;
}
