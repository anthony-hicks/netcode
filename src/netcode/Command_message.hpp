#pragma once

#include <chrono>

struct Client_message {
    std::chrono::duration<double> duration;
    uint32_t sequence_number;
};
