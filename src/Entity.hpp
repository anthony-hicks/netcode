#pragma once

#include <chrono>
#include <vector>

// TODO: Rename
struct Entity {
    struct Update {
        double x;
        std::chrono::system_clock::time_point t;
    };

    double position{0.0};
    std::vector<Update> updates;
};