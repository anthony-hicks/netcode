#include "Client.hpp"

#include "Utils.hpp"

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
            spdlog::info(
              "[client] recv: (seq={}, pos={:.3f})",
              msg.message.last_processed_sequence_number,
              msg.message.position
            );

            _offset = msg.message.position;

            // Remove acknowledged messages
            std::erase_if(
              _unacknowledged_messages,
              [msg](const Client_message& sent_msg) {
                  return sent_msg.sequence_number <=
                    msg.message.last_processed_sequence_number;
              }
            );

            // Reapply unacknowledged messages
            for (const Client_message& unack_msg : _unacknowledged_messages) {
                spdlog::debug(
                  "[client] reapply: (seq={}, dur={:.3f})",
                  unack_msg.sequence_number,
                  unack_msg.duration.count()
                );
                _offset = update_position(_offset, unack_msg.duration.count());
            }
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

void Client::save(const Client_message& msg)
{
    _unacknowledged_messages.push_back(msg);
}
