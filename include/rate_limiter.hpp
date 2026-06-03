#pragma once
#include <unordered_map>
#include <chrono>
#include <mutex>

class RateLimiter {
public:
    bool allow(const std::string& ip, int limit = 100, int window_sec = 60) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto now = std::chrono::steady_clock::now();
        auto& entry = m_clients[ip];
        if (entry.first + std::chrono::seconds(window_sec) < now) {
            entry = {now, 0};
        }
        if (entry.second >= limit) return false;
        entry.second++;
        return true;
    }
private:
    std::unordered_map<std::string, std::pair<std::chrono::steady_clock::time_point, int>> m_clients;
    std::mutex m_mutex;
};
