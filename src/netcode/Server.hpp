#pragma once

#include "Client.hpp"
#include "Command_message.hpp"
#include "common.hpp"

#include <mutex>
#include <queue>
#include <vector>

// TODO: docs

class Server {
    struct Message {
        Client_message message;
        std::chrono::system_clock::time_point recv_timestamp;
    };

    std::vector<Client*> _clients;
    std::mutex _queue_mutex;
    std::vector<Message> _queue;
    std::chrono::milliseconds _network_delay;

    State_message _state{.position = 0, .last_processed_sequence_number = 0};

public:
    explicit Server(std::chrono::milliseconds network_delay);
    std::size_t connect(Client* client);
    void send(Client_message const& msg, std::chrono::milliseconds delay);
    void update();
};
