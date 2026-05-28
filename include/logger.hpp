#pragma once

#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "colors.hpp"

/**
 * @brief Класс для логирования в консоль и файл одновременно
 */
class Logger {
public:
    static Logger& instance() {
        static Logger log;
        return log;
    }
    
    void init(const std::string& filename = "server.log") {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_file.is_open()) m_file.close();
        m_file.open(filename, std::ios::app);
        m_enabled = true;
    }
    
    void info(const std::string& tag, const std::string& msg) {
        log("INFO", tag, msg, COLOR_GREEN);
    }
    
    void warn(const std::string& tag, const std::string& msg) {
        log("WARN", tag, msg, COLOR_YELLOW);
    }
    
    void error(const std::string& tag, const std::string& msg) {
        log("ERROR", tag, msg, COLOR_RED);
    }
    
    void debug(const std::string& tag, const std::string& msg) {
        log("DEBUG", tag, msg, COLOR_CYAN);
    }
    
    ~Logger() {
        if (m_file.is_open()) m_file.close();
    }

private:
    Logger() : m_enabled(false) {}
    
    void log(const std::string& level, const std::string& tag, const std::string& msg, const char* color) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        std::tm tm;
        localtime_s(&tm, &time_t);
        
        char time_buf[20];
        std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm);
        
        // Форматируем сообщение
        std::stringstream ss;
        ss << "[" << time_buf << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
           << "[" << level << "] [" << tag << "] " << msg;
        
        // В консоль с цветом
        if (m_enabled) {
            std::cout << color << ss.str() << COLOR_RESET << std::endl;
        }
        
        // В файл без цвета
        if (m_file.is_open()) {
            m_file << ss.str() << std::endl;
            m_file.flush();
        }
    }
    
    std::ofstream m_file;
    std::mutex m_mutex;
    bool m_enabled;
};

#define LOG_INFO(tag, msg) Logger::instance().info(tag, msg)
#define LOG_WARN(tag, msg) Logger::instance().warn(tag, msg)
#define LOG_ERROR(tag, msg) Logger::instance().error(tag, msg)
#define LOG_DEBUG(tag, msg) Logger::instance().debug(tag, msg)
