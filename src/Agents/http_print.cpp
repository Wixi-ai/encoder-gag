#include "../../include/agents/http_agent.hpp"
#include "../../include/colors.hpp"
#include "../../include/logger.hpp"
#include <iostream>
#include <iomanip>

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
              << "  |  URL: http://" << std::left << std::setw(54) << (m_host + ":" + std::to_string(m_port)) << "          |\n"
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
              << "  |    curl --noproxy \"localhost\" http://" << m_host << ":" << m_port << "/health                   |\n"
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
