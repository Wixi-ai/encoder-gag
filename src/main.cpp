#include <so_5/all.hpp>
#include "agents/db_agent.hpp"
#include "agents/http_agent.hpp"
#include <thread>
#include <chrono>
#include <windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    std::cout << "========================================" << std::endl;
    std::cout << "  encoders_gag server starting..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    so_5::launch([](so_5::environment_t& env) {
        // Создаём DB агента (без аргументов)
        auto db_mbox = env.introduce_coop([](so_5::coop_t& coop) {
            return coop.make_agent<db_agent_t>()->so_direct_mbox();
        });
        
        // Создаём HTTP агента с передачей db_mbox
        env.introduce_coop([&](so_5::coop_t& coop) {
            coop.make_agent<http_agent_t>(db_mbox);
        });
        
        std::cout << "[MAIN] Agents created" << std::endl;
        
        // Тестовые сообщения DB агенту
        so_5::send<msg_hello>(db_mbox, msg_hello{"from main"});
        so_5::send<msg_bye>(db_mbox, msg_bye{"from main"});
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Тестовый POST запрос
        std::cout << "\n[MAIN] Sending test POST request..." << std::endl;
        httplib::Client client("localhost", 8080);
        auto post_res = client.Post("/api/v1/records", "{\"test\": \"data\"}", "application/json");
        
        if (post_res) {
            std::cout << "[MAIN] POST response: " << post_res->body << std::endl;
        } else {
            std::cout << "[MAIN] POST failed" << std::endl;
        }
        
        // Тестовый GET запрос
        std::cout << "\n[MAIN] Sending test GET request..." << std::endl;
        auto get_res = client.Get("/api/v1/records");
        if (get_res) {
            std::cout << "[MAIN] GET response: " << get_res->body << std::endl;
        } else {
            std::cout << "[MAIN] GET failed" << std::endl;
        }
        
        std::cout << "\n[MAIN] Server is running. Press Ctrl+C to exit..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(30));
    });
    
    return 0;
}
