/**
 * @file http_agent.cpp
 * @brief HTTP серверный агент - обрабатывает входящие REST запросы
 */

#include "../../include/agents/http_agent.hpp"
#include "../../include/colors.hpp"
#include "../../include/utils.hpp"
#include "../../include/logger.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
#include <sstream>

using json = nlohmann::json;

static bool validatePostRequest(const json &j, httplib::Response &res)
{
    if (!j.contains("id") || !j["id"].is_string())
    {
        res.set_content("{\"error\": \"missing or invalid 'id' field\"}", "application/json");
        res.status = 400;
        return false;
    }

    if (!is_valid_uuid(j["id"].get<std::string>()))
    {
        res.set_content("{\"error\": \"invalid uuid format\"}", "application/json");
        res.status = 400;
        return false;
    }

    if (!j.contains("block_size") || !j["block_size"].is_number_integer())
    {
        res.set_content("{\"error\": \"missing or invalid 'block_size' field\"}", "application/json");
        res.status = 400;
        return false;
    }

    if (j["block_size"].get<int>() <= 0)
    {
        res.set_content("{\"error\": \"block_size must be positive integer\"}", "application/json");
        res.status = 400;
        return false;
    }

    if (!j.contains("fblock") || !j["fblock"].is_string())
    {
        res.set_content("{\"error\": \"missing or invalid 'fblock' field\"}", "application/json");
        res.status = 400;
        return false;
    }

    if (!j.contains("streams") || !j["streams"].is_array())
    {
        res.set_content("{\"error\": \"missing or invalid 'streams' field (must be array)\"}", "application/json");
        res.status = 400;
        return false;
    }

    for (const auto &stream : j["streams"])
    {
        if (!stream.contains("id") || !stream["id"].is_number_integer())
        {
            res.set_content("{\"error\": \"each stream must have integer 'id' field\"}", "application/json");
            res.status = 400;
            return false;
        }
        if (!stream.contains("codec") || !stream["codec"].is_string())
        {
            res.set_content("{\"error\": \"each stream must have string 'codec' field\"}", "application/json");
            res.status = 400;
            return false;
        }
        if (stream.contains("width") && !stream["width"].is_number_integer())
        {
            res.set_content("{\"error\": \"width must be integer\"}", "application/json");
            res.status = 400;
            return false;
        }
        if (stream.contains("height") && !stream["height"].is_number_integer())
        {
            res.set_content("{\"error\": \"height must be integer\"}", "application/json");
            res.status = 400;
            return false;
        }
    }
    return true;
}

static std::chrono::steady_clock::time_point start_time;

// Константы
const int DEFAULT_LIMIT = 10;
const int MAX_LIMIT = 100;
const int TIMEOUT_SECONDS = 5;
const int DB_TIMEOUT_SECONDS = 5;

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
                   {
        if (req.body.empty()) {
            res.set_content("{\"error\": \"empty request body\"}", "application/json");
            res.status = 400;
            printResponse(400, "{\"error\": \"empty request body\"}");
            LOG_WARN("HTTP", "Empty request body");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!validatePostRequest(j, res)) {
                printResponse(res.status, res.body);
                return;
            }

            std::string id = j["id"].get<std::string>();
            // Берём file_path из запроса, если нет — используем значение по умолчанию
            std::string file_path = j.contains("file_path") ? j["file_path"].get<std::string>() : "C:/default/video.mp4";

            m_request_counter++;
            printRequest("POST", "/api/v1/records", m_request_counter, id.substr(0, 8), req.body);
            LOG_INFO("HTTP", "POST #" + std::to_string(m_request_counter) + " id=" + id + " path=" + file_path);

            auto promise = std::make_shared<std::promise<bool>>();
            auto future = promise->get_future();
            { std::lock_guard<std::mutex> lock(m_creates_mutex); m_pending_creates[id] = promise; }

            msg_create_record msg;
            msg.id = id;
            msg.file_path = file_path;
            msg.request_body = req.body;
            msg.reply_to = so_direct_mbox();
            so_5::send<msg_create_record>(m_db_mbox, msg);

            std::cout << COLOR_HTTP << "  -> DB Agent | sent, waiting for response" << COLOR_RESET << std::endl;

            if (future.wait_for(std::chrono::seconds(TIMEOUT_SECONDS)) != std::future_status::ready) {
                std::cout << COLOR_RED << "  [ERROR] Timeout waiting for DB response" << COLOR_RESET << std::endl;
                res.set_content("{\"error\": \"timeout\"}", "application/json");
                res.status = 504;
                printResponse(504, "{\"error\": \"timeout\"}");
                LOG_ERROR("HTTP", "Timeout for id=" + id);
                return;
            }

            if (future.get()) {
                std::string response = "{\"status\": \"created\", \"id\": \"" + id + "\", \"file_path\": \"" + file_path + "\"}";
                res.set_content(response, "application/json");
                res.status = 201;
                printResponse(201, response);
                LOG_INFO("HTTP", "Record created: " + id);
            } else {
                res.set_content("{\"error\": \"record already exists\"}", "application/json");
                res.status = 409;
                printResponse(409, "{\"error\": \"record already exists\"}");
                LOG_WARN("HTTP", "Duplicate record: " + id);
            }
        } catch (const json::parse_error& e) {
            res.set_content("{\"error\": \"invalid JSON format\"}", "application/json");
            res.status = 400;
            printResponse(400, "{\"error\": \"invalid JSON format\"}");
            LOG_WARN("HTTP", "Invalid JSON");
        } });

    // GET /api/v1/records с поддержкой пагинации и сортировки
    m_server->Get(R"(/api/v1/records)", [this](const httplib::Request &req, httplib::Response &res)
                  {
        int limit = DEFAULT_LIMIT;
        int offset = 0;
        std::string sort_by = "created_at";
        std::string sort_order = "asc";

        if (req.has_param("limit")) {
            limit = std::stoi(req.get_param_value("limit"));
            if (limit <= 0) limit = DEFAULT_LIMIT;
            if (limit > MAX_LIMIT) limit = MAX_LIMIT;
        }
        if (req.has_param("offset")) {
            offset = std::stoi(req.get_param_value("offset"));
            if (offset < 0) offset = 0;
        }
        if (req.has_param("sort_by")) {
            sort_by = req.get_param_value("sort_by");
            if (sort_by != "id" && sort_by != "file_path" && sort_by != "created_at") {
                sort_by = "created_at";
            }
        }
        if (req.has_param("sort_order")) {
            sort_order = req.get_param_value("sort_order");
            if (sort_order != "asc" && sort_order != "desc") {
                sort_order = "asc";
            }
        }

        int req_id = ++m_request_id_counter;
        printRequest("GET", "/api/v1/records?limit=" + std::to_string(limit) + "&offset=" + std::to_string(offset)
                     + "&sort_by=" + sort_by + "&sort_order=" + sort_order, req_id, "", "");
        LOG_DEBUG("HTTP", "GET all records #" + std::to_string(req_id) + " limit=" + std::to_string(limit)
                  + " offset=" + std::to_string(offset) + " sort_by=" + sort_by + " sort_order=" + sort_order);

        auto promise = std::make_shared<std::promise<msg_get_records_response>>();
        auto future = promise->get_future();
        { std::lock_guard<std::mutex> lock(m_pending_mutex); m_pending_requests[req_id] = promise; }

        so_5::send<msg_get_records>(m_db_mbox, req_id, limit, offset, sort_by, sort_order, so_direct_mbox());

        if (future.wait_for(std::chrono::seconds(DB_TIMEOUT_SECONDS)) != std::future_status::ready) {
            res.set_content("{\"error\": \"timeout\"}", "application/json");
            res.status = 504;
            printResponse(504, "{\"error\": \"timeout\"}");
            LOG_ERROR("HTTP", "Timeout getting all records");
            return;
        }

        auto response = future.get();
        json j = json::array();
        for (const auto& rec : response.records) {
            j.push_back({{"id", rec.first}, {"file_path", rec.second}});
        }

        json result = {
            {"total", response.total},
            {"limit", response.limit},
            {"offset", response.offset},
            {"sort_by", sort_by},
            {"sort_order", sort_order},
            {"records", j}
        };

        res.set_content(result.dump(), "application/json");
        res.status = 200;
        printResponse(200, result.dump().substr(0, 60) + (result.dump().size() > 60 ? "..." : ""));
        LOG_INFO("HTTP", "Returned " + std::to_string(response.records.size()) + " records (total: " + std::to_string(response.total) + ")"); });

    m_server->Get(R"(/api/v1/records/([a-f0-9-]+))", [this](const httplib::Request &req, httplib::Response &res)
                  {
        std::string id = req.matches[1];
        if (!is_valid_uuid(id)) {
            res.set_content("{\"error\": \"invalid uuid format\"}", "application/json");
            res.status = 400;
            printResponse(400, "{\"error\": \"invalid uuid format\"}");
            LOG_WARN("HTTP", "Invalid UUID: " + id);
            return;
        }

        int req_id = ++m_request_id_counter;
        printRequest("GET", "/api/v1/records/" + id, req_id, id.substr(0, 8), "");
        LOG_DEBUG("HTTP", "GET record by id: " + id);

        auto promise = std::make_shared<std::promise<msg_get_record_by_id_response>>();
        auto future = promise->get_future();
        { std::lock_guard<std::mutex> lock(m_pending_record_mutex); m_pending_record_requests[req_id] = promise; }

        msg_get_record_by_id msg;
        msg.id = id;
        msg.request_id = req_id;
        msg.reply_to = so_direct_mbox();
        so_5::send<msg_get_record_by_id>(m_db_mbox, msg);

        if (future.wait_for(std::chrono::seconds(DB_TIMEOUT_SECONDS)) != std::future_status::ready) {
            res.set_content("{\"error\": \"timeout\"}", "application/json");
            res.status = 504;
            printResponse(504, "{\"error\": \"timeout\"}");
            LOG_ERROR("HTTP", "Timeout getting record: " + id);
            return;
        }

        auto response = future.get();
        if (response.found) {
            json j = {{"id", response.id}, {"file_path", response.file_path}, {"created_at", response.created_at}};
            res.set_content(j.dump(), "application/json");
            res.status = 200;
            printResponse(200, j.dump());
            LOG_INFO("HTTP", "Record found: " + id);
        } else {
            res.set_content("{\"error\": \"record not found\"}", "application/json");
            res.status = 404;
            printResponse(404, "{\"error\": \"record not found\"}");
            LOG_WARN("HTTP", "Record not found: " + id);
        } });

    m_server->Delete(R"(/api/v1/records/([a-f0-9-]+))", [this](const httplib::Request &req, httplib::Response &res)
                     {
        std::string id = req.matches[1];
        if (!is_valid_uuid(id)) {
            res.set_content("{\"error\": \"invalid uuid format\"}", "application/json");
            res.status = 400;
            printResponse(400, "{\"error\": \"invalid uuid format\"}");
            LOG_WARN("HTTP", "Invalid UUID for delete: " + id);
            return;
        }

        int req_id = ++m_request_id_counter;
        printRequest("DELETE", "/api/v1/records/" + id, req_id, id.substr(0, 8), "");
        LOG_DEBUG("HTTP", "DELETE record: " + id);

        auto promise = std::make_shared<std::promise<msg_delete_record_by_id_response>>();
        auto future = promise->get_future();
        { std::lock_guard<std::mutex> lock(m_pending_delete_mutex); m_pending_delete_requests[req_id] = promise; }

        msg_delete_record_by_id msg;
        msg.id = id;
        msg.request_id = req_id;
        msg.reply_to = so_direct_mbox();
        so_5::send<msg_delete_record_by_id>(m_db_mbox, msg);

        if (future.wait_for(std::chrono::seconds(DB_TIMEOUT_SECONDS)) != std::future_status::ready) {
            res.set_content("{\"error\": \"timeout\"}", "application/json");
            res.status = 504;
            printResponse(504, "{\"error\": \"timeout\"}");
            LOG_ERROR("HTTP", "Timeout deleting record: " + id);
            return;
        }

        auto response = future.get();
        if (response.success) {
            res.set_content("{\"status\": \"deleted\", \"id\": \"" + id + "\"}", "application/json");
            res.status = 200;
            printResponse(200, "{\"status\": \"deleted\"}");
            LOG_INFO("HTTP", "Record deleted: " + id);
        } else {
            res.set_content("{\"error\": \"record not found\"}", "application/json");
            res.status = 404;
            printResponse(404, "{\"error\": \"record not found\"}");
            LOG_WARN("HTTP", "Record not found for delete: " + id);
        } });

    m_server->Get("/health", [this](const httplib::Request &, httplib::Response &res)
                  {
        auto now = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

        json j = {
            {"status", "ok"},
            {"version", "1.0.0"},
            {"uptime_seconds", uptime},
            {"requests_total", m_request_counter},
            {"active_requests", m_pending_requests.size() + m_pending_creates.size() + m_pending_delete_requests.size()},
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

void http_agent_t::printStartupInfo()
{
    std::cout << COLOR_MAIN << "\n"
              << "  +--------------------------------------------------------------------------------+\n"
              << "  ||                            ENCODERS_GAG v1.0                                 ||\n"
              << "  ||                     Server for Rigel Archive Module                          ||\n"
              << "  ||                                                                              ||\n"
              << "  ||                     HTTP: " << std::left << std::setw(42) << (m_host + ":" + std::to_string(m_port)) << "         ||\n"
              << "  ||                     API:  /api/v1/records                                    ||\n"
              << "  +--------------------------------------------------------------------------------+\n"
              << COLOR_RESET << std::endl;

    std::cout << COLOR_GREEN << "\n"
              << "  +------------------------------------------------------------------------------+\n"
              << "  |  [OK] SERVER STARTED SUCCESSFULLY                                            |\n"
              << "  +------------------------------------------------------------------------------+\n"
              << "  |  URL: http://" << std::left << std::setw(54) << (m_host + ":" + std::to_string(m_port)) << "        |\n"
              << "  +------------------------------------------------------------------------------+\n"
              << COLOR_RESET;

    std::cout << COLOR_CYAN << "\n"
              << "  +------------------------------------------------------------------------------+\n"
              << "  |  AVAILABLE ENDPOINTS:                                                        |\n"
              << "  |                                                                              |\n"
              << "  |    POST   /api/v1/records         - Create new record                        |\n"
              << "  |    GET    /api/v1/records         - Get all records (pagination + sorting)   |\n"
              << "  |    GET    /api/v1/records/{id}    - Get record by ID                         |\n"
              << "  |    DELETE /api/v1/records/{id}    - Delete record by ID                      |\n"
              << "  |    GET    /health                 - Health check                             |\n"
              << "  +------------------------------------------------------------------------------+\n"
              << COLOR_RESET;

    std::cout << COLOR_YELLOW << "\n"
              << "  +------------------------------------------------------------------------------+\n"
              << "  |  TEST COMMANDS (run in another terminal):                                    |\n"
              << "  |                                                                              |\n"
              << "  |    # Health check                                                            |\n"
              << "  |    curl --noproxy \"localhost\" http://" << m_host << ":" << m_port << "/health         |\n"
              << "  |                                                                              |\n"
              << "  |    # Create record with file path                                            |\n"
              << "  |    ./scripts/create.sh <uuid> </path/to/file>                                |\n"
              << "  |                                                                              |\n"
              << "  |    # Get all records (sorted)                                                |\n"
              << "  |    ./scripts/get_sorted.sh [sort_by] [sort_order] [limit] [offset]           |\n"
              << "  |                                                                              |\n"
              << "  |    # Get record by ID                                                        |\n"
              << "  |    ./scripts/get_by_id.sh <uuid>                                             |\n"
              << "  |                                                                              |\n"
              << "  |    # Delete record by ID                                                     |\n"
              << "  |    ./scripts/delete.sh <uuid>                                                |\n"
              << "  |                                                                              |\n"
              << "  +------------------------------------------------------------------------------+\n"
              << COLOR_RESET << std::endl;
}

void http_agent_t::printRequest(const std::string &method, const std::string &path, int num, const std::string &id, const std::string &body)
{
    std::cout << COLOR_HTTP << "\n  +-----------------------------------------------------------+\n"
              << "  | " << std::left << std::setw(55) << (method + " #" + std::to_string(num) + " | " + path) << " |\n"
              << "  +-----------------------------------------------------------+\n";
    if (!id.empty())
    {
        std::cout << "  | ID:   " << std::left << std::setw(52) << id << " |\n";
    }
    if (!body.empty())
    {
        std::string short_body = body.size() > 45 ? body.substr(0, 42) + "..." : body;
        std::cout << "  | Body: " << std::left << std::setw(51) << short_body << " |\n";
    }
    std::cout << "  +-----------------------------------------------------------+\n"
              << COLOR_RESET;
}

void http_agent_t::printResponse(int status, const std::string &body)
{
    std::string color = (status >= 200 && status < 300) ? COLOR_GREEN : COLOR_RED;
    std::cout << color << "  <- Response | Status: " << status << "\n"
              << "  <- Body     | " << (body.size() > 55 ? body.substr(0, 52) + "..." : body) << "\n"
              << COLOR_RESET;
}
