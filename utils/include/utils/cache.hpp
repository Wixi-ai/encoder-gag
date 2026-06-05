#pragma once

#include <unordered_map>
#include <string>
#include <chrono>
#include <mutex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

template<typename T>
class TimedCache {
public:
    TimedCache(int ttl_seconds = 60) : m_ttl(ttl_seconds) {}
    
    void set(const std::string& key, const T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache[key] = {value, std::chrono::steady_clock::now()};
    }
    
    bool get(const std::string& key, T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            auto age = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - it->second.timestamp).count();
            if (age < m_ttl) {
                value = it->second.value;
                return true;
            } else {
                m_cache.erase(it);
            }
        }
        return false;
    }
    
    void invalidate(const std::string& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache.erase(key);
    }
    
    void invalidateAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache.clear();
    }

private:
    struct CacheEntry {
        T value;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    std::unordered_map<std::string, CacheEntry> m_cache;
    std::mutex m_mutex;
    int m_ttl;
};
