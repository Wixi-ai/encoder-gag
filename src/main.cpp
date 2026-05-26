#include <so_5/all.hpp>
#include "../include/agents/db_agent.hpp"
#include "../include/agents/http_agent.hpp"
#include "../include/colors.hpp"
#include "../include/logger.hpp"
#include <thread>
#include <chrono>

int main() {
    // Инициализируем логгер
    Logger::instance().init("encoders_gag.log");
    
    LOG_INFO("MAIN", "Starting encoders_gag server v1.0");
    
    std::cout << COLOR_MAIN << "========================================" << COLOR_RESET << std::endl;
    std::cout << COLOR_MAIN << "  ENCODERS_GAG v1.0 STARTING" << COLOR_RESET << std::endl;
    std::cout << COLOR_MAIN << "========================================" << COLOR_RESET << std::endl;
    
    so_5::launch([](so_5::environment_t& env) {
        auto db_mbox = env.introduce_coop([](so_5::coop_t& coop) {
            return coop.make_agent<db_agent_t>()->so_direct_mbox();
        });

        env.introduce_coop([&](so_5::coop_t& coop) {
            coop.make_agent<http_agent_t>(db_mbox);
        });

        LOG_INFO("MAIN", "Agents created successfully");
        
        std::cout << COLOR_MAIN << "========================================" << COLOR_RESET << std::endl;
        std::cout << COLOR_MAIN << "  Server ready. Press Ctrl+C to exit" << COLOR_RESET << std::endl;
        std::cout << COLOR_MAIN << "========================================" << COLOR_RESET << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(3600));
    });
    
    LOG_INFO("MAIN", "Server stopped");
    return 0;
}
