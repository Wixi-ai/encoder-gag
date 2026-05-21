#ifndef HTTP_AGENT_HPP
#define HTTP_AGENT_HPP

#include <so_5/all.hpp>
#include <httplib.h>
#include <iostream>
#include <thread>
#include "../../messages.hpp"

class http_agent_t : public so_5::agent_t
{
public:
  http_agent_t(context_t ctx, so_5::mbox_t db_mbox)
      : so_5::agent_t{std::move(ctx)}, m_db_mbox{std::move(db_mbox)}
  {
  }

  void so_evt_start() override
  {
    std::cout << "[HTTP Agent] Сервер запущен на порту 8080" << std::endl;

    m_server = std::make_unique<httplib::Server>();

    // Обработчик POST запросов
    m_server->Post("/api/v1/records", [this](const httplib::Request &req, httplib::Response &res)
                   {
            std::cout << "[HTTP Agent] Получен POST запрос" << std::endl;

            // Создаём сообщение для DB агента
            msg_create_record msg;
            msg.id = "123e4567-e89b-12d3-a456-426614174000";  // временный ID
            msg.file_path = "C:/test/video.mp4";               // временный путь
            msg.request_body = req.body;                        // тело запроса

            // Отправляем сообщение DB агенту
            so_5::send<msg_create_record>(m_db_mbox, msg);

            std::cout << "[HTTP Agent] Отправлено msg_create_record DB агенту" << std::endl;

            // Отвечаем клиенту
            res.set_content("{\"status\": \"created\", \"id\": \"" + msg.id + "\"}", "application/json");
            res.status = 201; });

    m_server_thread = std::thread([this]()
                                  { m_server->listen("localhost", 8080); });
  }

  void so_evt_finish() override
  {
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
