#include <iostream>
#include <so_5/all.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 1. Структура сообщения (наше "письмо")
struct msg_create_record
{
  int id;
  std::string path;
};

// 2. Класс агента (наш "работник")
class db_agent_t : public so_5::agent_t
{
public:
  using so_5::agent_t::agent_t;

  void so_define_agent() override
  {
    // Подписываемся на сообщения типа msg_create_record
    so_subscribe_self().event([](const msg_create_record &msg)
                              {
            std::cout << "\n[DB Agent] Сообщение успешно получено!" << std::endl;
            std::cout << " > ID записи: " << msg.id << std::endl;
            std::cout << " > Путь к файлу: " << msg.path << std::endl; });
  }
};

int main()
{
  try
  {
    // Запускаем среду SObjectizer
    so_5::launch([](so_5::environment_t &env)
                 {
                   // 3. Регистрируем агента и получаем его "почтовый адрес" (mbox)
                   auto db_mbox = env.introduce_coop([](so_5::coop_t &coop)
                                                     {
                auto agent = coop.make_agent<db_agent_t>();
                return agent->so_direct_mbox(); });

                   // 4. Отправляем сообщение самому себе
                   std::cout << "--- Отправка сообщения агенту... ---" << std::endl;
                   so_5::send<msg_create_record>(db_mbox, 101, "C:/movies/video.mp4");

                   std::cout << "--- Система работает. Нажмите Ctrl+C для выхода ---" << std::endl;

                   // Мы НЕ вызываем env.stop(), чтобы программа не закрылась мгновенно.
                   // В реальном проекте здесь будет работать сервер или цикл ожидания.
                 });
  }
  catch (const std::exception &ex)
  {
    std::cerr << "Ошибка: " << ex.what() << std::endl;
  }

  return 0;
}
