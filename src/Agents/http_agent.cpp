#include "../../include/agents/http_agent.hpp"
#include "../../include/colors.hpp"
#include "../../include/utils.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

http_agent_t::http_agent_t(context_t ctx, so_5::mbox_t db_mbox)
    : so_5::agent_t{std::move(ctx)}
    , m_db_mbox{std::move(db_mbox)}
    , m_request_counter(0)
    , m_request_id_counter(0)
{}

void http_agent_t::so_evt_start() {
    std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] Server started on port 8080" << COLOR_RESET << std::endl;
    
    so_subscribe_self().event([this](const msg_get_records_response& response) {
        std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] Received response #" << response.request_id << COLOR_RESET << std::endl;
        
        auto it = m_pending_requests.find(response.request_id);
        if (it != m_pending_requests.end()) {
            it->second->set_value(response);
            std::lock_guard<std::mutex> lock(m_pending_mutex);
            m_pending_requests.erase(it);
        }
    });
    
    m_server = std::make_unique<httplib::Server>();
    
    m_server->Post("/api/v1/records", [this](const httplib::Request& req, httplib::Response& res) {
        m_request_counter++;
        std::string uuid = generate_uuid();
        
        std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] POST #" << m_request_counter << " id: " << uuid.substr(0, 8) << COLOR_RESET << std::endl;
        
        msg_create_record msg;
        msg.id = uuid;
        msg.file_path = "C:/test/video.mp4";
        msg.request_body = req.body;
        
        so_5::send<msg_create_record>(m_db_mbox, msg);
        
        std::string response = "{\"status\": \"created\", \"id\": \"" + uuid + "\"}";
        res.set_content(response, "application/json");
        res.status = 201;
    });
    
    m_server->Get("/api/v1/records", [this](const httplib::Request& req, httplib::Response& res) {
        int request_id = ++m_request_id_counter;
        std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] GET /records #" << request_id << COLOR_RESET << std::endl;
        
        auto promise = std::make_shared<std::promise<msg_get_records_response>>();
        auto future = promise->get_future();
        
        {
            std::lock_guard<std::mutex> lock(m_pending_mutex);
            m_pending_requests[request_id] = promise;
        }
        
        so_5::send<msg_get_records>(m_db_mbox, request_id, so_direct_mbox());
        
        auto status = future.wait_for(std::chrono::seconds(5));
        if (status == std::future_status::ready) {
            auto response = future.get();
            json j = json::array();
            for (const auto& rec : response.records) {
                j.push_back({{"id", rec.first}, {"file_path", rec.second}});
            }
            res.set_content(j.dump(), "application/json");
            res.status = 200;
        } else {
            std::cout << COLOR_HTTP << "  ERROR: timeout" << COLOR_RESET << std::endl;
            res.set_content("{\"error\": \"timeout\"}", "application/json");
            res.status = 504;
        }
    });
    
    m_server->Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("OK", "text/plain");
        res.status = 200;
    });
    
    m_server_thread = std::thread([this]() {
        m_server->listen("localhost", 8080);
    });
}

void http_agent_t::so_evt_finish() {
    std::cout << COLOR_HTTP << "[" << current_time() << "] [HTTP] Shutting down..." << COLOR_RESET << std::endl;
    if (m_server) {
        m_server->stop();
    }
    if (m_server_thread.joinable()) {
        m_server_thread.join();
    }
}
