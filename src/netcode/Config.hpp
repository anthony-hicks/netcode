//
// Created by ahicks on 11/27/23.
//

#ifndef NETCODE_CONFIG_HPP
#define NETCODE_CONFIG_HPP

#include "common.hpp"

#include <chrono>

class Config {
    float _server_update_rate_hz{1.67F};
    float _client_update_rate_hz{20.0F};

    bool _prediction{false};
    bool _reconciliation{false};
    bool _interpolation{false};

    std::chrono::milliseconds _latency{250};

public:

    bool& prediction() { return _prediction; }
    bool& reconciliation() { return _reconciliation; }
    bool& interpolation() { return _interpolation; }
    std::chrono::milliseconds latency() const { return _latency; }
    float server_update_rate() const { return _server_update_rate_hz; }
    float client_update_rate() const { return _client_update_rate_hz; }
    void server_update_rate(float hz) { _server_update_rate_hz = hz; }
    void client_update_rate(float hz) { _client_update_rate_hz = hz; }

    milliseconds_d server_update_interval() const
    {
        return seconds_d{1.0F / _server_update_rate_hz};
    }

    milliseconds_d client_update_interval() const
    {
        return seconds_d{1.0F / _client_update_rate_hz};
    }

    void latency(std::chrono::milliseconds latency) { _latency = latency; }
};

#endif  // NETCODE_CONFIG_HPP
