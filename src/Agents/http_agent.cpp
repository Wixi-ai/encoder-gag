#include "../../include/agents/http_agent.hpp"
#include "../../include/colors.hpp"
#include "../../include/utils.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

http_agent_t::http_agent_t(context_t ctx, so_5::mbox_t db_mbox)
    : so_5::agent_t{std::move(ctx)}, m_db_mbox{std::move(db_mbox)}, m_request_counter(0), m_request_id_counter(0)
{
}

void http_agent_t::so_evt_start()
{
    printStartupInfo();

    so_subscribe_self().event([this](const msg_create_response& response) {
        auto it = m_pending_creates.find(response.id);
        if (it != m_pending_creates.end()) {
            it->second->set_value(response.success);
            std::lock_guard<std::mutex> lock(m_creates_mutex);
            m_pending_creates.erase(it);
        }
    });

    so_subscribe_self().event([this](const msg_get_records_response& response) {
        auto it = m_pending_requests.find(response.request_id);
        if (it != m_pending_requests.end()) {
            it->second->set_value(response);
            std::lock_guard<std::mutex> lock(m_pending_mutex);
            m_pending_requests.erase(it);
        }
    });
    
    so_subscribe_self().event([this](const msg_get_record_by_id_response& response) {
        auto it = m_pending_record_requests.find(response.request_id);
        if (it != m_pending_record_requests.end()) {
            it->second->set_value(response);
            std::lock_guard<std::mutex> lock(m_pending_record_mutex);
            m_pending_record_requests.erase(it);
        }
    });
    
    so_subscribe_self().event([this](const msg_delete_record_by_id_response& response) {
        auto it = m_pending_delete_requests.find(response.request_id);
        if (it != m_pending_delete_requests.end()) {
            it->second->set_value(response);
            std::lock_guard<std::mutex> lock(m_pending_delete_mutex);
            m_pending_delete_requests.erase(it);
        }
    });

    m_server = std::make_unique<httplib::Server>();

    m_server->Post("/api/v1/records", [this](const httplib::Request &req, httplib::Response &res) {
        m_request_counter++;
        std::string uuid = generate_uuid();
        printRequest("POST", "/api/v1/records", m_request_counter, uuid.substr(0, 8), req.body);

        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();
        { std::lock_guard<std::mutex> lock(m_creates_mutex); m_pending_creates[uuid] = promise; }

        msg_create_record msg;
        msg.id = uuid;
        msg.file_path = "C:/test/video.mp4";
        msg.request_body = req.body;
        msg.reply_to = so_direct_mbox();
        so_5::send<msg_create_record>(m_db_mbox, msg);
        
        std::cout << COLOR_HTTP << "  -> DB Agent | sent, waiting for response" << COLOR_RESET << std::endl;
        
        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::ready && future.get()) {
            std::string response = "{\"status\": \"created\", \"id\": \"" + uuid + "\"}";
            res.set_content(response, "application/json");
            res.status = 201;
            printResponse(201, response);
        } else {
            std::cout << COLOR_RED << "  [ERROR] Failed to save record or timeout" << COLOR_RESET << std::endl;
            res.set_content("{\"error\": \"failed to save record\"}", "application/json");
            res.status = 500;
            printResponse(500, "{\"error\": \"failed to save record\"}");
        }
    });

    m_server->Get(R"(/api/v1/records)", [this](const httplib::Request &, httplib::Response &res) {
        int request_id = ++m_request_id_counter;
        printRequest("GET", "/api/v1/records", request_id, "", "");

        auto promise = std::make_shared<std::promise<msg_get_records_response>>();
        auto future = promise->get_future();
        { std::lock_guard<std::mutex> lock(m_pending_mutex); m_pending_requests[request_id] = promise; }

        so_5::send<msg_get_records>(m_db_mbox, request_id, so_direct_mbox());
        
        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
            auto response = future.get();
            json j = json::array();
            for (const auto& rec : response.records) {
                j.push_back({{"id", rec.first}, {"file_path", rec.second}});
            }
            res.set_content(j.dump(), "application/json");
            res.status = 200;
            printResponse(200, j.dump().substr(0, 60) + (j.dump().size() > 60 ? "..." : ""));
        } else {
            res.set_content("{\"error\": \"timeout\"}", "application/json");
            res.status = 504;
            printResponse(504, "{\"error\": \"timeout\"}");
        }
    });
    
    m_server->Get(R"(/api/v1/records/(\w+-\w+-\w+-\w+-\w+))", [this](const httplib::Request &req, httplib::Response &res) {
        std::string id = req.matches[1];
        int request_id = ++m_request_id_counter;
        printRequest("GET", "/api/v1/records/" + id, request_id, id.substr(0, 8), "");

        auto promise = std::make_shared<std::promise<msg_get_record_by_id_response>>();
        auto future = promise->get_future();
        { std::lock_guard<std::mutex> lock(m_pending_record_mutex); m_pending_record_requests[request_id] = promise; }

        msg_get_record_by_id msg = {id, request_id, so_direct_mbox()};
        so_5::send<msg_get_record_by_id>(m_db_mbox, msg);

        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
            auto response = future.get();
            if (response.found) {
                json j = {{"id", response.id}, {"file_path", response.file_path}, {"created_at", response.created_at}};
                res.set_content(j.dump(), "application/json");
                res.status = 200;
                printResponse(200, j.dump());
            } else {
                res.set_content("{\"error\": \"record not found\"}", "application/json");
                res.status = 404;
                printResponse(404, "{\"error\": \"record not found\"}");
            }
        } else {
            res.set_content("{\"error\": \"timeout\"}", "application/json");
            res.status = 504;
            printResponse(504, "{\"error\": \"timeout\"}");
        }
    });
    
    m_server->Delete(R"(/api/v1/records/(\w+-\w+-\w+-\w+-\w+))", [this](const httplib::Request &req, httplib::Response &res) {
        std::string id = req.matches[1];
        int request_id = ++m_request_id_counter;
        printRequest("DELETE", "/api/v1/records/" + id, request_id, id.substr(0, 8), "");

        auto promise = std::make_shared<std::promise<msg_delete_record_by_id_response>>();
        auto future = promise->get_future();
        { std::lock_guard<std::mutex> lock(m_pending_delete_mutex); m_pending_delete_requests[request_id] = promise; }

        msg_delete_record_by_id msg = {id, request_id, so_direct_mbox()};
        so_5::send<msg_delete_record_by_id>(m_db_mbox, msg);

        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
            auto response = future.get();
            if (response.success) {
                res.set_content("{\"status\": \"deleted\", \"id\": \"" + id + "\"}", "application/json");
                res.status = 200;
                printResponse(200, "{\"status\": \"deleted\"}");
            } else {
                res.set_content("{\"error\": \"record not found\"}", "application/json");
                res.status = 404;
                printResponse(404, "{\"error\": \"record not found\"}");
            }
        } else {
            res.set_content("{\"error\": \"timeout\"}", "application/json");
            res.status = 504;
            printResponse(504, "{\"error\": \"timeout\"}");
        }
    });

    m_server->Get("/health", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("OK", "text/plain");
        res.status = 200;
    });

    m_server_thread = std::thread([this]() { m_server->listen("localhost", 8080); });
}

void http_agent_t::so_evt_finish()
{
    if (m_server) { m_server->stop(); }
    if (m_server_thread.joinable()) { m_server_thread.join(); }
}

void http_agent_t::printStartupInfo()
{
    std::cout << COLOR_MAIN << R"(
  ================================================================
  ||                         ENCODERS_GAG v1.0                  ||
  ||                  Server for Rigel Archive Module           ||
  ||                                                            ||
  ||                  HTTP: localhost:8080                      ||
  ||                  API:  /api/v1/records                     ||
  ================================================================
)" << COLOR_RESET << std::endl;

    std::cout << COLOR_GREEN << R"(
  +-----------------------------------------------------------+
  |  [OK] SERVER STARTED SUCCESSFULLY                         |
  +-----------------------------------------------------------+
  |  URL: http://localhost:8080                               |
  |                                                           |
  |  AVAILABLE ENDPOINTS:                                     |
  |    POST   /api/v1/records         - Create new record     |
  |    GET    /api/v1/records         - Get all records       |
  |    GET    /api/v1/records/{id}    - Get record by ID      |
  |    DELETE /api/v1/records/{id}    - Delete record by ID   |
  |    GET    /health                 - Health check          |
  +-----------------------------------------------------------+
)" << COLOR_RESET;

    std::cout << COLOR_MAIN << R"(
  +------------------------------------------------------------------------------+
  |  TEST COMMANDS (run in another terminal):                                    |
  |                                                                              |
  |    # Health check                                                            |
  |    ./scripts/health.sh                                                       |
  |                                                                              |
  |    # Create record                                                           |
  |    ./scripts/create.sh                                                       |
  |                                                                              |
  |    # Get all records                                                         |
  |    ./scripts/get.sh                                                          |
  |                                                                              |
  |    # Get record by ID                                                        |
  |    ./scripts/get_by_id.sh <uuid>                                             |
  |                                                                              |
  |    # Delete record by ID                                                     |
  |    ./scripts/delete.sh <uuid>                                                |
  |                                                                              |
  +------------------------------------------------------------------------------+
)" << COLOR_RESET << std::endl;
}

void http_agent_t::printRequest(const std::string &method, const std::string &path, int num, const std::string &id, const std::string &body)
{
    std::cout << COLOR_HTTP << "\n  +-----------------------------------------------------------+\n"
              << "  | " << std::left << std::setw(55) << (method + " #" + std::to_string(num) + " | " + path) << " |\n"
              << "  +-----------------------------------------------------------+\n";
    if (!id.empty()) {
        std::cout << "  | ID:   " << std::left << std::setw(52) << id << " |\n";
    }
    if (!body.empty()) {
        std::string short_body = body.size() > 45 ? body.substr(0, 42) + "..." : body;
        std::cout << "  | Body: " << std::left << std::setw(51) << short_body << " |\n";
    }
    std::cout << "  +-----------------------------------------------------------+\n" << COLOR_RESET;
}

void http_agent_t::printResponse(int status, const std::string &body)
{
    std::string color = (status >= 200 && status < 300) ? COLOR_GREEN : COLOR_RED;
    std::cout << color << "  <- Response | Status: " << status << "\n"
              << "  <- Body     | " << (body.size() > 55 ? body.substr(0, 52) + "..." : body) << "\n" << COLOR_RESET;
}
