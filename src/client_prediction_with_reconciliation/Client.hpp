#pragma once

#include "common.hpp"
#include "Command_message.hpp"
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

public:
    void offset(double);
    [[nodiscard]] double offset() const;

    void process_server_messages();
    void send(State_message const& msg, std::chrono::milliseconds delay);
    void save(Client_message const& msg);
};
