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
        // Создаём DB агента (теперь без передачи mbox)
        auto db_mbox = env.introduce_coop([](so_5::coop_t& coop) {
            return coop.make_agent<db_agent_t>()->so_direct_mbox();
        });
        
        // Создаём HTTP агента (передаём mbox DB агента)
        env.introduce_coop([&](so_5::coop_t& coop) {
            coop.make_agent<http_agent_t>(db_mbox);
        });
        
        // Тестовые сообщения
        so_5::send<msg_hello>(db_mbox, msg_hello{"from main"});
        so_5::send<msg_bye>(db_mbox, msg_bye{"from main"});
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Тестовый HTTP запрос
        std::cout << "\n[MAIN] Sending test HTTP request..." << std::endl;
        httplib::Client client("localhost", 8080);
        auto res = client.Post("/api/v1/records", "{\"test\": \"data\"}", "application/json");
        
        if (res) {
            std::cout << "[MAIN] Server response: " << res->body << std::endl;
        } else {
            std::cout << "[MAIN] Failed to send request" << std::endl;
        }
        
        std::cout << "\n[MAIN] Server is running. Press Ctrl+C to exit..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(30));
    });
    
    return 0;
}
