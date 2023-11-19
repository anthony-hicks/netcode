#pragma once

#include "Command_message.hpp"
#include "common.hpp"
#include "State_message.hpp"

#include <mutex>
#include <queue>

class Client {
    struct Message {
        State_message message;
        std::chrono::system_clock::time_point recv_timestamp;
    };

    std::mutex _queue_mutex;
    std::vector<Message> _queue;

    double _offset{0.0};

    std::vector<Client_message> _unacknowledged_messages;

    std::size_t _id;

    struct Update {
        double x;
        std::chrono::system_clock::time_point t;
    };

    std::vector<Update> _updates;

public:
    void offset(double);

    [[nodiscard]]
    double offset() const;

    void id(std::size_t id) { _id = id; }

    void process_server_messages();
    void send(State_message const& msg, std::chrono::milliseconds delay);
    void save(Client_message const& msg);
    void interpolate_entities(milliseconds_d server_update_interval, std::size_t delay_in_ticks);

private:
    [[nodiscard]] std::optional<std::pair<Update, Update>>
    find_surrounding_updates(
      std::chrono::time_point<std::chrono::system_clock, milliseconds_d> time) const;
};
