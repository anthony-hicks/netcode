#pragma once

#include <stdexcept>

enum class Command {
    move_left,
    move_right
};

struct Command_message {
    Command command;
};

inline const char* to_string(Command command) {
    switch (command) {
        case Command::move_left:
            return "move left";
        case Command::move_right:
            return "move right";
        default:
            throw std::runtime_error("invalid enum");
    }
}
