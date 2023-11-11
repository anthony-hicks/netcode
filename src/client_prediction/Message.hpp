#pragma once

#include <cstddef>
#include <stdexcept>

enum class Command {
    move_left,
    move_right
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

struct Command_Message {
    Command command;

    [[nodiscard]] std::size_t constexpr size() const {
        return sizeof(command);
    }

    const char* to_string() const {
        return ::to_string(command);
    }
};

struct Position_Message {
    int position;

    [[nodiscard]] std::size_t constexpr size() const {
        return sizeof(position);
    }
};