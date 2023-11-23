#pragma once

[[nodiscard]]
inline double update_position(double position, double delta_time)
{
    static constexpr int pixels_per_second{500};
    return position + pixels_per_second * delta_time;
}
