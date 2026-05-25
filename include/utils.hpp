#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <random>

inline std::string current_time() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm tm;
    localtime_s(&tm, &time_t);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    std::stringstream ss;
    ss << buf << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

inline std::string generate_uuid() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            ss << '-';
        } else if (i == 14) {
            ss << dis2(gen);
        } else {
            ss << dis(gen);
        }
    }
    return ss.str();
}

#endif
