#include <so_5/all.hpp>
#include "agents/db_agent.hpp"
#include "agents/http_agent.hpp"
#include <thread>
#include <chrono>
#include <httplib.h> // Добавляем для клиента
#include <windows.h>

int main()
{
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);

  so_5::launch([](so_5::environment_t &env)
               {
        // Создаём DB агента
        auto db_mbox = env.introduce_coop([](so_5::coop_t& coop) {
            return coop.make_agent<db_agent_t>()->so_direct_mbox();
        });

        // Создаём HTTP агента (сервер)
        env.introduce_coop([&](so_5::coop_t& coop) {
            coop.make_agent<http_agent_t>(db_mbox);
        });

        // Отправляем тестовые сообщения
        so_5::send<msg_hello>(db_mbox, msg_hello{"from main"});
        so_5::send<msg_bye>(db_mbox, msg_bye{"from main"});

        // Ждём, пока сервер запустится
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // ===== НОВЫЙ КОД: САМИ ОТПРАВЛЯЕМ ЗАПРОС =====
        std::cout << "\n========== ОТПРАВКА ТЕСТОВОГО ЗАПРОСА ==========" << std::endl;

        // Создаём HTTP клиент
        httplib::Client client("localhost", 8080);

        // Отправляем POST запрос
        auto res = client.Post("/api/v1/records", "test", "text/plain");

        // Выводим ответ
        if (res) {
            std::cout << "Ответ от сервера: " << res->body << std::endl;
            std::cout << "Статус: " << res->status << std::endl;
        } else {
            std::cout << "Ошибка: не удалось отправить запрос" << std::endl;
        }

        std::cout << "================================================\n" << std::endl;

        // Держим программу открытой, чтобы увидеть логи
        std::this_thread::sleep_for(std::chrono::seconds(5)); });
  return 0;
}
