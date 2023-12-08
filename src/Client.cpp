#include "Client.hpp"

#include "Utils.hpp"

#include <spdlog/spdlog.h>

void Client::send(const Server_update& update, std::chrono::milliseconds delay)
{
    const Delayed_server_update msg{
      .message = update, .recv_timestamp = std::chrono::system_clock::now() + delay};

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
            for (const auto& state : msg.message.states) {
                spdlog::info(
                  "[client] [{}] recv: (seq={}) (id={}, pos={:.3f})",
                  _entity_id,
                  msg.message.last_processed_input,
                  state.id,
                  state.position
                );

                // If we haven't seen this entity before, allocate space for it
                if (state.id + 1 > _entities.size()) {
                    // Since we use indices to match entity ids, we need to resize
                    // the vector to hold at least the (id + 1) so we can index
                    // into that entity. Alternatively, we could preallocate
                    // or use a different data structure like a flat map.
                    _entities.resize(state.id + 1);
                }

                _entities[state.id].position = state.position;

                // If the entity is not this client's entity, save server update
                // for interpolation
                if (state.id != _entity_id) {
                    const auto now = std::chrono::system_clock::now();

                    _entities[state.id].updates.emplace_back(state.position, now);
                    continue;
                }

                // TODO: Global config object / DI? Assuming rn that
                //  this just never grows

                // Remove acknowledged messages
                std::erase_if(
                  _unacknowledged_messages,
                  [msg](const Client_message& sent_msg) {
                      return sent_msg.sequence_number <=
                        msg.message.last_processed_input;
                  }
                );

                // Reapply unacknowledged messages
                for (const Client_message& unack_msg : _unacknowledged_messages) {
                    spdlog::debug(
                      "[client] reapply: (seq={}, dur={:.3f})",
                      unack_msg.sequence_number,
                      unack_msg.duration.count()
                    );
                    offset(update_position(offset(), unack_msg.duration.count()));
                }
            }
        }
    }

    std::erase_if(_queue, [](const auto& msg) {
        return msg.recv_timestamp <= std::chrono::system_clock::now();
    });
}

std::optional<std::pair<Entity::Update, Entity::Update>>
Client::find_surrounding_updates(
  const std::vector<Entity::Update>& updates,
  std::chrono::time_point<std::chrono::system_clock, milliseconds_d> time
) const
{
    // TODO: Could be a small class, or a free function and pass in update container
    if (updates.size() < 2) {
        return std::nullopt;
    }

    for (std::size_t i = 0; i < updates.size() - 1; ++i) {
        auto u0 = updates[i];
        auto u1 = updates[i + 1];

        if (u0.t <= time && time <= u1.t) {
            return std::make_pair(u0, u1);
        }
    }

    return std::nullopt;
}

void Client::interpolate_entities(
  const milliseconds_d server_update_interval, const std::size_t delay_in_ticks
)
{
    const auto delay = delay_in_ticks * server_update_interval;

    const auto now = std::chrono::system_clock::now();

    // We want to render other entities in the past
    const auto render_time = now - delay;

    for (auto& entity : _entities) {
        // Must have at least the number of ticks we want to delay rendering by + 1,
        // so we have an update prior the render time that we could interpolate from.
        if (entity.updates.size() > delay_in_ticks) {
            std::optional<std::pair<Entity::Update, Entity::Update>>
              surrounding_updates =
                find_surrounding_updates(entity.updates, render_time);

            if (surrounding_updates.has_value()) {
                auto [u0, u1] = surrounding_updates.value();
                entity.position =
                  std::lerp(u0.x, u1.x, (render_time - u0.t) / (u1.t - u0.t));

                // Remove all updates that occurred before our "t0"
                std::erase_if(entity.updates, [u0](const Entity::Update& update) {
                    return update.t < u0.t;
                });
            }
        }
    }
}

double Client::offset() const
{
    // TODO: Hacky and just so we can query where the spectator thinks
    //  the player position is using the same offset() API. Spectator
    //  should probably just be something different entirely. The
    //  spectator also implicitly has a player character that is never
    //  rendered.

    if (_entities.empty()) {
        return 0.0;
    }

    return _entities[0].position;
}

void Client::offset(double offset)
{
    if (_entities.empty()) {
        return;
    }

    _entities[0].position = offset;
}

void Client::save(const Client_message& msg)
{
    _unacknowledged_messages.push_back(msg);
}
