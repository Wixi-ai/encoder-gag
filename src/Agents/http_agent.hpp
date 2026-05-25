#ifndef HTTP_AGENT_HPP
#define HTTP_AGENT_HPP

#include <so_5/all.hpp>
#include <httplib.h>
#include <iostream>
#include <thread>
#include <random>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include "../../messages.hpp"
#include "../colors.hpp"
#include "../utils.hpp"

using json = nlohmann::json;

// Вспомогательная структура для хранения ответа
struct PendingRequest {
    std::promise<msg_get_records_response> promise;
    std::atomic<bool> completed{false};
};

class http_agent_t : public so_5::agent_t {
public:
    http_agent_t(context_t ctx, so_5::mbox_t db_mbox)
        : so_5::agent_t{std::move(ctx)}
        , m_db_mbox{std::move(db_mbox)}
        , m_request_counter(0)
        , m_request_id_counter(0)
    {}

    void so_evt_start() override {
        std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] Server started on port 8080" << COLOR_RESET << std::endl;
        
        // Подписываемся на ответы от DB агента
        so_subscribe_self().event([this](const msg_get_records_response& response) {
            std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] Received response from DB" << COLOR_RESET << std::endl;
            
            // Ищем ожидающий запрос по ID
            auto it = m_pending_requests.find(response.request_id);
            if (it != m_pending_requests.end()) {
                it->second->promise.set_value(response);
                it->second->completed = true;
                std::lock_guard<std::mutex> lock(m_pending_mutex);
                m_pending_requests.erase(it);
            }
        });
        
        m_server = std::make_unique<httplib::Server>();
        
        // POST /api/v1/records
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
        
        // GET /api/v1/records
        m_server->Get("/api/v1/records", [this](const httplib::Request& req, httplib::Response& res) {
            int request_id = ++m_request_id_counter;
            std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] GET /records request #" << request_id << COLOR_RESET << std::endl;
            
            try {
                // Создаём promise/future для синхронного ожидания
                auto pending = std::make_shared<PendingRequest>();
                std::future<msg_get_records_response> future = pending->promise.get_future();
                
                // Сохраняем в карту ожидающих запросов
                {
                    std::lock_guard<std::mutex> lock(m_pending_mutex);
                    m_pending_requests[request_id] = pending;
                }
                
                // Отправляем запрос в DB-агент с ID для обратной связи
                so_5::send<msg_get_records>(m_db_mbox, request_id, so_direct_mbox());
                std::cout << COLOR_HTTP << "  -> sent request #" << request_id << " to DB agent" << COLOR_RESET << std::endl;
                
                // Ждём ответ с таймаутом 5 секунд
                auto status = future.wait_for(std::chrono::seconds(5));
                if (status == std::future_status::ready) {
                    auto response = future.get();
                    std::cout << COLOR_HTTP << "  <- got response with " << response.records.size() << " records" << COLOR_RESET << std::endl;
                    
                    json j = json::array();
                    for (const auto& rec : response.records) {
                        json item;
                        item["id"] = rec.first;
                        item["file_path"] = rec.second;
                        j.push_back(item);
                    }
                    
                    res.set_content(j.dump(), "application/json");
                    res.status = 200;
                } else {
                    std::cout << COLOR_HTTP << "  ERROR: timeout waiting for response #" << request_id << COLOR_RESET << std::endl;
                    res.set_content("{\"error\": \"timeout\"}", "application/json");
                    res.status = 504;
                }
            } catch (const std::exception& e) {
                std::cout << COLOR_HTTP << "  ERROR: " << e.what() << COLOR_RESET << std::endl;
                res.set_content("{\"error\": \"" + std::string(e.what()) + "\"}", "application/json");
                res.status = 500;
            }
        });
        
        m_server->Get("/health", [](const httplib::Request&, httplib::Response& res) {
            res.set_content("OK", "text/plain");
            res.status = 200;
            std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] GET /health" << COLOR_RESET << std::endl;
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
    std::atomic<int> m_request_id_counter;
    std::unordered_map<int, std::shared_ptr<PendingRequest>> m_pending_requests;
    std::mutex m_pending_mutex;
};

#endif
