#ifndef HTTP_AGENT_HPP
#define HTTP_AGENT_HPP

#include <so_5/all.hpp>
#include <httplib.h>
#include <iostream>

class http_agent_t : public so_5::agent_t
{
public:
  http_agent_t(context_t ctx, so_5::mbox_t db_mbox)
      : so_5::agent_t{std::move(ctx)}, m_db_mbox{std::move(db_mbox)}
  {
  }

  void so_evt_start() override
  {
    std::cout << "[HTTP Agent] Запуск сервера на порту 8080..." << std::endl;

    m_server = std::make_unique<httplib::Server>();

    // Временно: просто отвечаем "OK" на любой POST запрос
    m_server->Post("/api/v1/records", [](const httplib::Request &req, httplib::Response &res)
                   {
            std::cout << "[HTTP Agent] Получен POST /api/v1/records" << std::endl;
            res.set_content("{\"status\": \"ok\"}", "application/json");
            res.status = 201; });

    // Запускаем сервер в отдельном потоке
    m_server_thread = std::thread([this]()
                                  { m_server->listen("localhost", 8080); });
  }

  void so_evt_finish() override
  {
    std::cout << "[HTTP Agent] Остановка сервера..." << std::endl;
    if (m_server)
    {
      m_server->stop();
    }
    if (m_server_thread.joinable())
    {
      m_server_thread.join();
    }
  }

private:
  so_5::mbox_t m_db_mbox;
  std::unique_ptr<httplib::Server> m_server;
  std::thread m_server_thread;
};

#endif
