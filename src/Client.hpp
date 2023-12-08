#pragma once

#include "Command_message.hpp"
#include "common.hpp"
#include "Entity.hpp"
#include "Server_update.hpp"

#include <mutex>
#include <queue>

class Client {
    struct Delayed_server_update {
        Server_update message;
        std::chrono::system_clock::time_point recv_timestamp;
    };

    std::mutex _queue_mutex;
    std::vector<Delayed_server_update> _queue;

    std::vector<Client_message> _unacknowledged_messages;

    size_t _entity_id;

    std::vector<Entity> _entities;

public:
    void offset(double);

    [[nodiscard]]
    double offset() const;

    size_t entity_id() const { return _entity_id; }

    void entity_id(size_t id) { _entity_id = id; }

    void process_server_messages();
    void send(Server_update const& update, std::chrono::milliseconds delay);
    void save(Client_message const& msg);
    void interpolate_entities(
      milliseconds_d server_update_interval, std::size_t delay_in_ticks
    );

private:
    [[nodiscard]]
    std::optional<std::pair<Entity::Update, Entity::Update>>
    find_surrounding_updates(
      const std::vector<Entity::Update>& updates,
      std::chrono::time_point<std::chrono::system_clock, milliseconds_d> time
    ) const;
};
