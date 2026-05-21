#ifndef HTTP_AGENT_HPP
#define HTTP_AGENT_HPP

#include <so_5/all.hpp>
#include <httplib.h>
#include <iostream>
#include <thread>
#include <random>
#include <sstream>
#include <iomanip>
#include "../../messages.hpp"

inline std::string generate_uuid() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            ss << '-';
        } else if (i == 14) {
            ss << dis2(gen);
        } else {
            ss << dis(gen);
        }
    }
    return ss.str();
}

class http_agent_t : public so_5::agent_t {
public:
    http_agent_t(context_t ctx, so_5::mbox_t db_mbox)
        : so_5::agent_t{std::move(ctx)}
        , m_db_mbox{std::move(db_mbox)}
    {}

    void so_evt_start() override {
        std::cout << "[HTTP Agent] Server started on port 8080" << std::endl;
        
        m_server = std::make_unique<httplib::Server>();
        
        m_server->Post("/api/v1/records", [this](const httplib::Request& req, httplib::Response& res) {
            std::cout << "[HTTP Agent] POST /api/v1/records received" << std::endl;
            
            msg_create_record msg;
            msg.id = generate_uuid();
            msg.file_path = "C:/test/video.mp4";
            msg.request_body = req.body;
            
            so_5::send<msg_create_record>(m_db_mbox, msg);
            std::cout << "[HTTP Agent] Message sent to DB agent" << std::endl;
            
            std::string response = "{\"status\": \"created\", \"id\": \"" + msg.id + "\"}";
            res.set_content(response, "application/json");
            res.status = 201;
        });
        
        m_server->Get("/api/v1/records", [this](const httplib::Request&, httplib::Response& res) {
            std::cout << "[HTTP Agent] GET /api/v1/records received" << std::endl;
            res.set_content("[]", "application/json");
            res.status = 200;
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
};

#endif
