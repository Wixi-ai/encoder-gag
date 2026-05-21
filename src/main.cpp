#include <so_5/all.hpp>
#include "agents/db_agent.hpp"
#include "agents/http_agent.hpp"
#include "colors.hpp"
#include <thread>
#include <chrono>
#include <windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    std::cout << COLOR_MAIN << "========================================" << COLOR_RESET << std::endl;
    std::cout << COLOR_MAIN << "  encoders_gag server starting..." << COLOR_RESET << std::endl;
    std::cout << COLOR_MAIN << "========================================" << COLOR_RESET << std::endl;
    
    so_5::launch([](so_5::environment_t& env) {
        // Создаём DB агента
        auto db_mbox = env.introduce_coop([](so_5::coop_t& coop) {
            return coop.make_agent<db_agent_t>()->so_direct_mbox();
        });
        
        // Создаём HTTP агента с передачей db_mbox
        env.introduce_coop([&](so_5::coop_t& coop) {
            coop.make_agent<http_agent_t>(db_mbox);
        });
        
        std::cout << COLOR_MAIN << "[MAIN] Agents created" << COLOR_RESET << std::endl;
        
        // Тестовые сообщения DB агенту
        so_5::send<msg_hello>(db_mbox, msg_hello{"from main"});
        so_5::send<msg_bye>(db_mbox, msg_bye{"from main"});
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Тестовый POST запрос
        std::cout << COLOR_MAIN << "\n[MAIN] Sending test POST request..." << COLOR_RESET << std::endl;
        httplib::Client client("localhost", 8080);
        auto post_res = client.Post("/api/v1/records", "{\"test\": \"data\"}", "application/json");
        
        if (post_res) {
            std::cout << COLOR_MAIN << "[MAIN] POST response: " << post_res->body << COLOR_RESET << std::endl;
        } else {
            std::cout << COLOR_MAIN << "[MAIN] POST failed" << COLOR_RESET << std::endl;
        }
        
        // Тестовый GET запрос
        std::cout << COLOR_MAIN << "\n[MAIN] Sending test GET request..." << COLOR_RESET << std::endl;
        auto get_res = client.Get("/api/v1/records");
        if (get_res) {
            std::cout << COLOR_MAIN << "[MAIN] GET response: " << get_res->body << COLOR_RESET << std::endl;
        } else {
            std::cout << COLOR_MAIN << "[MAIN] GET failed" << COLOR_RESET << std::endl;
        }
        
        std::cout << COLOR_MAIN << "\n[MAIN] Server is running. Press Ctrl+C to exit..." << COLOR_RESET << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(30));
    });
    
    return 0;
}
