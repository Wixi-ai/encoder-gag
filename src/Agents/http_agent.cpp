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

    m_server = std::make_unique<httplib::Server>();

    m_server->Post("/api/v1/records", [this](const httplib::Request &req, httplib::Response &res)
                   {
        m_request_counter++;
        std::string uuid = generate_uuid();

        printRequest("POST", "/api/v1/records", m_request_counter, uuid.substr(0, 8), req.body);

        msg_create_record msg;
        msg.id = uuid;
        msg.file_path = "C:/test/video.mp4";
        msg.request_body = req.body;

        so_5::send<msg_create_record>(m_db_mbox, msg);
        std::cout << COLOR_HTTP << "  -> DB Agent | sent" << COLOR_RESET << std::endl;

        std::string response = "{\"status\": \"created\", \"id\": \"" + uuid + "\"}";
        res.set_content(response, "application/json");
        res.status = 201;

        printResponse(201, response); });

    m_server->Get("/api/v1/records", [this](const httplib::Request &req, httplib::Response &res)
                  {
        int request_id = ++m_request_id_counter;
        printRequest("GET", "/api/v1/records", request_id, "", "");

        auto promise = std::make_shared<std::promise<msg_get_records_response>>();
        auto future = promise->get_future();

        {
            std::lock_guard<std::mutex> lock(m_pending_mutex);
            m_pending_requests[request_id] = promise;
        }

        std::cout << COLOR_HTTP << "  -> DB Agent | request #" << request_id << COLOR_RESET << std::endl;

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

            std::cout << COLOR_HTTP << "  <- DB Agent | response with " << response.records.size() << " records" << COLOR_RESET << std::endl;
            printResponse(200, j.dump().substr(0, 80) + (j.dump().size() > 80 ? "..." : ""));
        } else {
            std::cout << COLOR_RED << "  [ERROR] Timeout waiting for DB response" << COLOR_RESET << std::endl;
            res.set_content("{\"error\": \"timeout\"}", "application/json");
            res.status = 504;
        } });

    m_server->Get("/health", [](const httplib::Request &, httplib::Response &res)
                  {
        res.set_content("OK", "text/plain");
        res.status = 200; });

    m_server_thread = std::thread([this]()
                                  { m_server->listen("localhost", 8080); });
}

void http_agent_t::so_evt_finish()
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
  |    POST   /api/v1/records    - Create new record          |
  |    GET    /api/v1/records    - Get all records            |
  |    GET    /health            - Health check               |
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
  +------------------------------------------------------------------------------+
)" << COLOR_RESET << std::endl;
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
