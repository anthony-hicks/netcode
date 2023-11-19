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

            // If this client is not the player, save server messages for interpolation
            if (_id != 1) {
                const auto now = std::chrono::system_clock::now();
                _updates.emplace_back(msg.message.position, now);
            }

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

std::optional<std::pair<Client::Update, Client::Update>>
Client::find_surrounding_updates(
  std::chrono::time_point<std::chrono::system_clock, milliseconds_d> time) const
{
    // TODO: Could be a small class, or a free function and pass in update container
    if (_updates.size() < 2) {
        return std::nullopt;
    }

    for (std::size_t i = 0; i < _updates.size() - 1; ++i) {
        auto u0 = _updates[i];
        auto u1 = _updates[i + 1];

        if (u0.t <= time && time <= u1.t) {
            return std::make_pair(u0, u1);
        }
    }

    return std::nullopt;
}

void Client::interpolate_entities(const milliseconds_d server_update_interval)
{
    static constexpr int delay_ticks{1};
    const auto delay = delay_ticks * server_update_interval;

    const auto now = std::chrono::system_clock::now();

    // We want to render other entities in the past
    const auto render_time = now - delay;

    // Must have at least the number of ticks we want to delay rendering by + 1, so
    // we have an update prior the render time that we could interpolate from.
    if (_updates.size() > delay_ticks) {
        std::optional<std::pair<Update, Update>> surrounding_updates =
          find_surrounding_updates(render_time);

        if (surrounding_updates.has_value()) {
            auto [u0, u1] = surrounding_updates.value();
            _offset = std::lerp(u0.x, u1.x, (render_time - u0.t) / (u1.t - u0.t));

            // Remove all updates that occurred before our "t0"
            std::erase_if(_updates, [u0](const Update& update) {
                return update.t < u0.t;
            });
        }
    }
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
