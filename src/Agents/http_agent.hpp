#ifndef HTTP_AGENT_HPP
#define HTTP_AGENT_HPP

#include <so_5/all.hpp>
#include <httplib.h>
#include <iostream>
#include <thread>
#include "../../messages.hpp"
#include "../colors.hpp"
#include "../utils.hpp"

class http_agent_t : public so_5::agent_t {
public:
    http_agent_t(context_t ctx, so_5::mbox_t db_mbox)
        : so_5::agent_t{std::move(ctx)}
        , m_db_mbox{std::move(db_mbox)}
        , m_request_counter(0)
    {}

    void so_evt_start() override {
        std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] Server started on port 8080" << COLOR_RESET << std::endl;
        
        m_server = std::make_unique<httplib::Server>();
        
        m_server->Post("/api/v1/records", [this](const httplib::Request& req, httplib::Response& res) {
            m_request_counter++;
            std::string uuid = generate_uuid();
            
            std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] POST #" << m_request_counter << " id: " << uuid.substr(0, 8) << COLOR_RESET << std::endl;
            std::cout << COLOR_HTTP << "  body: " << req.body << COLOR_RESET << std::endl;
            
            msg_create_record msg;
            msg.id = uuid;
            msg.file_path = "C:/test/video.mp4";
            msg.request_body = req.body;
            
            so_5::send<msg_create_record>(m_db_mbox, msg);
            std::cout << COLOR_HTTP << "  -> sent to DB agent" << COLOR_RESET << std::endl;
            
            std::string response = "{\"status\": \"created\", \"id\": \"" + uuid + "\"}";
            res.set_content(response, "application/json");
            res.status = 201;
            
            std::cout << COLOR_HTTP << "  <- response: 201 Created" << COLOR_RESET << std::endl;
        });
        
        m_server->Get("/api/v1/records", [](const httplib::Request&, httplib::Response& res) {
            res.set_content("[]", "application/json");
            res.status = 200;
            std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] GET /records -> [] (mock)" << COLOR_RESET << std::endl;
        });
        
        m_server->Get("/health", [](const httplib::Request&, httplib::Response& res) {
            res.set_content("OK", "text/plain");
            res.status = 200;
        });
        
        m_server_thread = std::thread([this]() {
            m_server->listen("localhost", 8080);
        });
    }

    void so_evt_finish() override {
        std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] Shutting down..." << COLOR_RESET << std::endl;
        if (m_server) {
            m_server->stop();
        }
        if (m_server_thread.joinable()) {
            m_server_thread.join();
        }
    }

private:
    so_5::mbox_t m_db_mbox;
    std::unique_ptr<httplib::Server> m_server;
    std::thread m_server_thread;
    int m_request_counter;
};

#endif
