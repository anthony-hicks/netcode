#pragma once

#include <cstdint>

struct State_message {
    double position;
    uint32_t last_processed_sequence_number;
};