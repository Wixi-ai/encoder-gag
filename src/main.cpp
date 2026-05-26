#include <so_5/all.hpp>
#include "../include/agents/db_agent.hpp"
#include "../include/agents/http_agent.hpp"
#include "../include/colors.hpp"
#include "../include/logger.hpp"
#include <thread>
#include <chrono>
#include <cstdlib>
#include <csignal>
#include <atomic>

std::atomic<bool> g_running{true};
so_5::environment_t* g_env = nullptr;

void signal_handler(int signal) {
    LOG_INFO("MAIN", "Received signal " + std::to_string(signal) + ", shutting down...");
    std::cout << COLOR_YELLOW << "\n  Shutting down gracefully..." << COLOR_RESET << std::endl;
    g_running = false;
    if (g_env) {
        g_env->stop();
    }
}

std::string get_env(const std::string& key, const std::string& default_value) {
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : default_value;
}

int main() {
    // Регистрируем обработчик сигналов
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    std::string log_file = get_env("ENCODERS_GAG_LOG", "encoders_gag.log");
    std::string db_path = get_env("ENCODERS_GAG_DB", "records.db");
    int port = std::stoi(get_env("ENCODERS_GAG_PORT", "8080"));
    std::string host = get_env("ENCODERS_GAG_HOST", "localhost");
    
    Logger::instance().init(log_file);
    
    LOG_INFO("MAIN", "Starting encoders_gag server v1.0");
    LOG_INFO("MAIN", "Configuration: host=" + host + " port=" + std::to_string(port) + " db=" + db_path);
    
    std::cout << COLOR_MAIN << "========================================" << COLOR_RESET << std::endl;
    std::cout << COLOR_MAIN << "  ENCODERS_GAG v1.0 STARTING" << COLOR_RESET << std::endl;
    std::cout << COLOR_MAIN << "  Host: " << host << " Port: " << port << COLOR_RESET << std::endl;
    std::cout << COLOR_MAIN << "  Database: " << db_path << COLOR_RESET << std::endl;
    std::cout << COLOR_MAIN << "========================================" << COLOR_RESET << std::endl;
    
    so_5::launch([&](so_5::environment_t& env) {
        g_env = &env;
        
        auto db_mbox = env.introduce_coop([&](so_5::coop_t& coop) {
            return coop.make_agent<db_agent_t>(db_path)->so_direct_mbox();
        });

        env.introduce_coop([&](so_5::coop_t& coop) {
            coop.make_agent<http_agent_t>(db_mbox, host, port);
        });

        LOG_INFO("MAIN", "Agents created successfully");
        
        std::cout << COLOR_MAIN << "========================================" << COLOR_RESET << std::endl;
        std::cout << COLOR_MAIN << "  Server ready. Press Ctrl+C to exit" << COLOR_RESET << std::endl;
        std::cout << COLOR_MAIN << "========================================" << COLOR_RESET << std::endl;

        // Ждём сигнала завершения
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    LOG_INFO("MAIN", "Server stopped gracefully");
    std::cout << COLOR_GREEN << "  Server stopped successfully." << COLOR_RESET << std::endl;
    return 0;
}
