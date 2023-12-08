#pragma once

#include <chrono>

struct Client_message {
    std::size_t entity_id;
    std::chrono::duration<double> duration;
    uint32_t sequence_number;
};
