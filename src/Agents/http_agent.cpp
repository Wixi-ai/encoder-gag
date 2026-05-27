/**
 * @file http_agent.cpp
 * @brief HTTP серверный агент - основная логика
 */

#include "../../include/agents/http_agent.hpp"
#include "../../include/colors.hpp"
#include "../../include/utils.hpp"
#include "../../include/logger.hpp"
#include "../../include/constants.hpp"
#include <nlohmann/json.hpp>
#include <chrono>

using json = nlohmann::json;

std::chrono::steady_clock::time_point start_time;

http_agent_t::http_agent_t(context_t ctx, so_5::mbox_t db_mbox, const std::string &host, int port)
    : so_5::agent_t{std::move(ctx)}, m_db_mbox{std::move(db_mbox)}, m_request_counter(0), m_request_id_counter(0), m_host(host), m_port(port)
{
    LOG_INFO("HTTP", "HTTP Agent created with host=" + host + " port=" + std::to_string(port));
}

void http_agent_t::so_evt_start()
{
    start_time = std::chrono::steady_clock::now();
    printStartupInfo();
    LOG_INFO("HTTP", "Starting HTTP server on " + m_host + ":" + std::to_string(m_port));

    so_subscribe_self().event([this](const msg_create_response &response)
                              {
        auto it = m_pending_creates.find(response.id);
        if (it != m_pending_creates.end()) {
            it->second->set_value(response.success);
            std::lock_guard<std::mutex> lock(m_creates_mutex);
            m_pending_creates.erase(it);
        } });

    so_subscribe_self().event([this](const msg_get_records_response &response)
                              {
        auto it = m_pending_requests.find(response.request_id);
        if (it != m_pending_requests.end()) {
            it->second->set_value(response);
            std::lock_guard<std::mutex> lock(m_pending_mutex);
            m_pending_requests.erase(it);
        } });

    so_subscribe_self().event([this](const msg_get_record_by_id_response &response)
                              {
        auto it = m_pending_record_requests.find(response.request_id);
        if (it != m_pending_record_requests.end()) {
            it->second->set_value(response);
            std::lock_guard<std::mutex> lock(m_pending_record_mutex);
            m_pending_record_requests.erase(it);
        } });

    so_subscribe_self().event([this](const msg_delete_record_by_id_response &response)
                              {
        auto it = m_pending_delete_requests.find(response.request_id);
        if (it != m_pending_delete_requests.end()) {
            it->second->set_value(response);
            std::lock_guard<std::mutex> lock(m_pending_delete_mutex);
            m_pending_delete_requests.erase(it);
        } });

    m_server = std::make_unique<httplib::Server>();

    m_server->Post("/api/v1/records", [this](const httplib::Request &req, httplib::Response &res)
                   { handlePost(req, res); });

    m_server->Get(R"(/api/v1/records)", [this](const httplib::Request &req, httplib::Response &res)
                  { handleGetAll(req, res); });

    m_server->Get(R"(/api/v1/records/([a-f0-9-]+))", [this](const httplib::Request &req, httplib::Response &res)
                  { handleGetById(req.matches[1], res); });

    m_server->Delete(R"(/api/v1/records/([a-f0-9-]+))", [this](const httplib::Request &req, httplib::Response &res)
                     { handleDelete(req.matches[1], res); });

    m_server->Get("/health", [this](const httplib::Request &, httplib::Response &res)
                  {
        auto now = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

        json j = {
            {"status", "ok"},
            {"version", "1.0.0"},
            {"uptime_seconds", uptime},
            {"requests_total", m_request_counter},
            {"active_requests", static_cast<int>(m_pending_requests.size() + m_pending_creates.size() + m_pending_delete_requests.size())},
            {"host", m_host},
            {"port", m_port}
        };

        res.set_content(j.dump(), "application/json");
        res.status = 200;
        LOG_DEBUG("HTTP", "Health check requested"); });

    m_server_thread = std::thread([this]()
                                  { m_server->listen(m_host, m_port); });
}

void http_agent_t::so_evt_finish()
{
    LOG_INFO("HTTP", "Shutting down HTTP server");
    if (m_server)
    {
        m_server->stop();
        LOG_INFO("HTTP", "HTTP server stopped");
    }
    if (m_server_thread.joinable())
    {
        m_server_thread.join();
        LOG_INFO("HTTP", "Server thread joined");
    }
}
