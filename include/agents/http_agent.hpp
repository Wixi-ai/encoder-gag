#pragma once

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
#include "../messages.hpp"
#include "../colors.hpp"
#include "../utils.hpp"

class http_agent_t : public so_5::agent_t {
public:
    http_agent_t(context_t ctx, so_5::mbox_t db_mbox);
    void so_evt_start() override;
    void so_evt_finish() override;

private:
    void printStartupInfo();
    void printRequest(const std::string& method, const std::string& path, int num, const std::string& id, const std::string& body);
    void printResponse(int status, const std::string& body);

    so_5::mbox_t m_db_mbox;
    std::unique_ptr<httplib::Server> m_server;
    std::thread m_server_thread;
    int m_request_counter;
    std::atomic<int> m_request_id_counter;
    
    // Для GET /records (все записи)
    std::unordered_map<int, std::shared_ptr<std::promise<msg_get_records_response>>> m_pending_requests;
    std::mutex m_pending_mutex;
    
    // Для GET /records/{id} (одна запись)
    std::unordered_map<int, std::shared_ptr<std::promise<msg_get_record_by_id_response>>> m_pending_record_requests;
    std::mutex m_pending_record_mutex;
    
    // Для POST
    std::unordered_map<std::string, std::shared_ptr<std::promise<bool>>> m_pending_creates;
    std::mutex m_creates_mutex;
};
