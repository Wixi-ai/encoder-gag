#ifndef DB_AGENT_HPP
#define DB_AGENT_HPP

#include <so_5/all.hpp>
#include <iostream>
#include "../../messages.hpp"

class db_agent_t : public so_5::agent_t
{
public:
  using so_5::agent_t::agent_t;

  void so_define_agent() override
  {
    // Обработчик для msg_hello
    so_subscribe_self().event([](const msg_hello &msg)
                              { std::cout << "[DB Agent] Hello, " << msg.name << "!" << std::endl; });

    // Обработчик для msg_bye
    so_subscribe_self().event([](const msg_bye &msg)
                              { std::cout << "[DB Agent] Bye, " << msg.name << "!" << std::endl; });

    // Обработчик для msg_create_record
    so_subscribe_self().event([](const msg_create_record &msg)
                              {
            std::cout << "\n========== DB АГЕНТ ==========" << std::endl;
            std::cout << "Получено сообщение: msg_create_record" << std::endl;
            std::cout << "Данные:" << std::endl;
            std::cout << "  - id: " << msg.id << std::endl;
            std::cout << "  - file_path: " << msg.file_path << std::endl;
            std::cout << "  - request_body: " << msg.request_body << std::endl;
            std::cout << "==============================\n" << std::endl; });
  }
};

#endif
