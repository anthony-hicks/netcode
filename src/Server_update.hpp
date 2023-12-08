#pragma once

#include <cstdint>
#include <vector>

struct Entity_state {
    double position;
    size_t id;
};

struct Server_update {
    std::vector<Entity_state> states;
    uint32_t last_processed_input;
};