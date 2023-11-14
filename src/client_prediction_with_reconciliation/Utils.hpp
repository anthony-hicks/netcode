#ifndef NETCODE_UTILS_HPP
#define NETCODE_UTILS_HPP

[[nodiscard]]
inline double update_position(double position, double delta_time)
{
    static constexpr int pixels_per_second{50};
    return position + pixels_per_second * delta_time;
}

#endif  // NETCODE_UTILS_HPP
