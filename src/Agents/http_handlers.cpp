#include "../../include/agents/http_agent.hpp"
#include "../../include/colors.hpp"
#include "../../include/utils.hpp"
#include "../../include/logger.hpp"
#include "../../include/constants.hpp"
#include <nlohmann/json.hpp>
#include <chrono>

using json = nlohmann::json;

extern std::chrono::steady_clock::time_point start_time;

static bool validatePostRequest(const json &j, httplib::Response &res)
{
    if (!j.contains("id") || !j["id"].is_string())
    {
        res.set_content("{\"error\": \"missing or invalid 'id' field\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }

    if (!is_valid_uuid(j["id"].get<std::string>()))
    {
        res.set_content("{\"error\": \"invalid uuid format\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }

    if (!j.contains("block_size") || !j["block_size"].is_number_integer())
    {
        res.set_content("{\"error\": \"missing or invalid 'block_size' field\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }

    if (j["block_size"].get<int>() <= 0)
    {
        res.set_content("{\"error\": \"block_size must be positive integer\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }

    if (!j.contains("fblock") || !j["fblock"].is_string())
    {
        res.set_content("{\"error\": \"missing or invalid 'fblock' field\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }

    if (!j.contains("streams") || !j["streams"].is_array())
    {
        res.set_content("{\"error\": \"missing or invalid 'streams' field (must be array)\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }

    for (const auto &stream : j["streams"])
    {
        if (!stream.contains("id") || !stream["id"].is_number_integer())
        {
            res.set_content("{\"error\": \"each stream must have integer 'id' field\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
            return false;
        }
        if (!stream.contains("codec") || !stream["codec"].is_string())
        {
            res.set_content("{\"error\": \"each stream must have string 'codec' field\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
            return false;
        }
        if (stream.contains("width") && !stream["width"].is_number_integer())
        {
            res.set_content("{\"error\": \"width must be integer\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
            return false;
        }
        if (stream.contains("height") && !stream["height"].is_number_integer())
        {
            res.set_content("{\"error\": \"height must be integer\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
            return false;
        }
    }
    return true;
}

void http_agent_t::handlePost(const httplib::Request &req, httplib::Response &res)
{
    if (req.body.empty())
    {
        res.set_content("{\"error\": \"empty request body\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        printResponse(res.status, "{\"error\": \"empty request body\"}");
        LOG_WARN("HTTP", "Empty request body");
        return;
    }

    try
    {
        json j = json::parse(req.body);
        if (!validatePostRequest(j, res))
        {
            printResponse(res.status, res.body);
            return;
        }

        std::string id = j["id"].get<std::string>();
        std::string file_path = j.contains("file_path") ? j["file_path"].get<std::string>() : "C:/default/video.mp4";

        m_request_counter++;
        printRequest("POST", "/api/v1/records", m_request_counter, id.substr(0, 8), req.body);
        LOG_INFO("HTTP", "POST #" + std::to_string(m_request_counter) + " id=" + id + " path=" + file_path);

        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();
        {
            std::lock_guard<std::mutex> lock(m_creates_mutex);
            m_pending_creates[id] = promise;
        }

        msg_create_record msg;
        msg.id = id;
        msg.file_path = file_path;
        msg.request_body = req.body;
        msg.reply_to = so_direct_mbox();
        so_5::send<msg_create_record>(m_db_mbox, msg);

        std::cout << COLOR_HTTP << "  -> DB Agent | sent, waiting for response" << COLOR_RESET << std::endl;

        if (future.wait_for(std::chrono::seconds(config::TIMEOUT_SECONDS)) != std::future_status::ready)
        {
            std::cout << COLOR_RED << "  [ERROR] Timeout waiting for DB response" << COLOR_RESET << std::endl;
            res.set_content("{\"error\": \"timeout\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::GATEWAY_TIMEOUT);
            printResponse(res.status, "{\"error\": \"timeout\"}");
            LOG_ERROR("HTTP", "Timeout for id=" + id);
            return;
        }

        bool success = future.get();
        std::cout << COLOR_HTTP << "  <- DB Agent | success=" << success << COLOR_RESET << std::endl;

        if (success)
        {
            std::string response = "{\"status\": \"created\", \"id\": \"" + id + "\", \"file_path\": \"" + file_path + "\"}";
            res.set_content(response, "application/json");
            res.status = static_cast<int>(HttpStatus::CREATED);
            printResponse(res.status, response);
            LOG_INFO("HTTP", "Record created: " + id);
        }
        else
        {
            res.set_content("{\"error\": \"record already exists\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::CONFLICT);
            printResponse(res.status, "{\"error\": \"record already exists\"}");
            LOG_WARN("HTTP", "Duplicate record: " + id);
        }
    }
    catch (const json::parse_error &e)
    {
        res.set_content("{\"error\": \"invalid JSON format\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        printResponse(res.status, "{\"error\": \"invalid JSON format\"}");
        LOG_WARN("HTTP", "Invalid JSON");
    }
}

void http_agent_t::handleGetAll(const httplib::Request &req, httplib::Response &res)
{
    int limit = config::DEFAULT_LIMIT;
    int offset = 0;
    std::string sort_by = "created_at";
    std::string sort_order = "asc";
    
    // Параметры фильтрации
    std::string codec = "";
    std::string from_date = "";
    std::string to_date = "";
    std::string file_path = "";

    if (req.has_param("limit"))
    {
        limit = std::stoi(req.get_param_value("limit"));
        if (limit <= 0) limit = config::DEFAULT_LIMIT;
        if (limit > config::MAX_LIMIT) limit = config::MAX_LIMIT;
    }
    if (req.has_param("offset"))
    {
        offset = std::stoi(req.get_param_value("offset"));
        if (offset < 0) offset = 0;
    }
    if (req.has_param("sort_by"))
    {
        sort_by = req.get_param_value("sort_by");
        if (sort_by != "id" && sort_by != "file_path" && sort_by != "created_at")
            sort_by = "created_at";
    }
    if (req.has_param("sort_order"))
    {
        sort_order = req.get_param_value("sort_order");
        if (sort_order != "asc" && sort_order != "desc")
            sort_order = "asc";
    }
    
    // Парсим фильтры
    if (req.has_param("codec")) codec = req.get_param_value("codec");
    if (req.has_param("from_date")) from_date = req.get_param_value("from_date");
    if (req.has_param("to_date")) to_date = req.get_param_value("to_date");
    if (req.has_param("file_path")) file_path = req.get_param_value("file_path");

    int req_id = ++m_request_id_counter;
    
    std::string path = "/api/v1/records?limit=" + std::to_string(limit) + "&offset=" + std::to_string(offset) 
                     + "&sort_by=" + sort_by + "&sort_order=" + sort_order;
    if (!codec.empty()) path += "&codec=" + codec;
    if (!from_date.empty()) path += "&from_date=" + from_date;
    if (!to_date.empty()) path += "&to_date=" + to_date;
    if (!file_path.empty()) path += "&file_path=" + file_path;
    
    printRequest("GET", path, req_id, "", "");
    LOG_DEBUG("HTTP", "GET all records #" + std::to_string(req_id) + " limit=" + std::to_string(limit) 
              + " offset=" + std::to_string(offset) + " sort_by=" + sort_by + " sort_order=" + sort_order
              + " codec=" + codec + " from=" + from_date + " to=" + to_date + " path=" + file_path);

    auto promise = std::make_shared<std::promise<msg_get_records_response>>();
    auto future = promise->get_future();
    {
        std::lock_guard<std::mutex> lock(m_pending_mutex);
        m_pending_requests[req_id] = promise;
    }

    so_5::send<msg_get_records>(m_db_mbox, req_id, limit, offset, sort_by, sort_order, codec, from_date, to_date, file_path, so_direct_mbox());

    if (future.wait_for(std::chrono::seconds(config::DB_TIMEOUT_SECONDS)) != std::future_status::ready)
    {
        res.set_content("{\"error\": \"timeout\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::GATEWAY_TIMEOUT);
        printResponse(res.status, "{\"error\": \"timeout\"}");
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
    
    // Добавляем фильтры в ответ если они были использованы
    if (!codec.empty()) result["codec"] = codec;
    if (!from_date.empty()) result["from_date"] = from_date;
    if (!to_date.empty()) result["to_date"] = to_date;
    if (!file_path.empty()) result["file_path"] = file_path;
    
    res.set_content(result.dump(), "application/json");
    res.status = static_cast<int>(HttpStatus::OK);
    printResponse(res.status, result.dump().substr(0, 60) + (result.dump().size() > 60 ? "..." : ""));
    LOG_INFO("HTTP", "Returned " + std::to_string(response.records.size()) + " records (total: " + std::to_string(response.total) + ")");
}

void http_agent_t::handleGetById(const std::string &id, httplib::Response &res)
{
    if (!is_valid_uuid(id))
    {
        res.set_content("{\"error\": \"invalid uuid format\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        printResponse(res.status, "{\"error\": \"invalid uuid format\"}");
        LOG_WARN("HTTP", "Invalid UUID: " + id);
        return;
    }

    int req_id = ++m_request_id_counter;
    printRequest("GET", "/api/v1/records/" + id, req_id, id.substr(0, 8), "");
    LOG_DEBUG("HTTP", "GET record by id: " + id);

    auto promise = std::make_shared<std::promise<msg_get_record_by_id_response>>();
    auto future = promise->get_future();
    {
        std::lock_guard<std::mutex> lock(m_pending_record_mutex);
        m_pending_record_requests[req_id] = promise;
    }

    msg_get_record_by_id msg;
    msg.id = id;
    msg.request_id = req_id;
    msg.reply_to = so_direct_mbox();
    so_5::send<msg_get_record_by_id>(m_db_mbox, msg);

    if (future.wait_for(std::chrono::seconds(config::DB_TIMEOUT_SECONDS)) != std::future_status::ready)
    {
        res.set_content("{\"error\": \"timeout\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::GATEWAY_TIMEOUT);
        printResponse(res.status, "{\"error\": \"timeout\"}");
        LOG_ERROR("HTTP", "Timeout getting record: " + id);
        return;
    }

    auto response = future.get();
    if (response.found)
    {
        json j = {{"id", response.id}, {"file_path", response.file_path}, {"created_at", response.created_at}};
        res.set_content(j.dump(), "application/json");
        res.status = static_cast<int>(HttpStatus::OK);
        printResponse(res.status, j.dump());
        LOG_INFO("HTTP", "Record found: " + id);
    }
    else
    {
        res.set_content("{\"error\": \"record not found\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::NOT_FOUND);
        printResponse(res.status, "{\"error\": \"record not found\"}");
        LOG_WARN("HTTP", "Record not found: " + id);
    }
}

void http_agent_t::handleDelete(const std::string &id, httplib::Response &res)
{
    if (!is_valid_uuid(id))
    {
        res.set_content("{\"error\": \"invalid uuid format\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        printResponse(res.status, "{\"error\": \"invalid uuid format\"}");
        LOG_WARN("HTTP", "Invalid UUID for delete: " + id);
        return;
    }

    int req_id = ++m_request_id_counter;
    printRequest("DELETE", "/api/v1/records/" + id, req_id, id.substr(0, 8), "");
    LOG_DEBUG("HTTP", "DELETE record: " + id);

    auto promise = std::make_shared<std::promise<msg_delete_record_by_id_response>>();
    auto future = promise->get_future();
    {
        std::lock_guard<std::mutex> lock(m_pending_delete_mutex);
        m_pending_delete_requests[req_id] = promise;
    }

    msg_delete_record_by_id msg;
    msg.id = id;
    msg.request_id = req_id;
    msg.reply_to = so_direct_mbox();
    so_5::send<msg_delete_record_by_id>(m_db_mbox, msg);

    if (future.wait_for(std::chrono::seconds(config::DB_TIMEOUT_SECONDS)) != std::future_status::ready)
    {
        res.set_content("{\"error\": \"timeout\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::GATEWAY_TIMEOUT);
        printResponse(res.status, "{\"error\": \"timeout\"}");
        LOG_ERROR("HTTP", "Timeout deleting record: " + id);
        return;
    }

    auto response = future.get();
    if (response.success)
    {
        res.set_content("{\"status\": \"deleted\", \"id\": \"" + id + "\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::OK);
        printResponse(res.status, "{\"status\": \"deleted\"}");
        LOG_INFO("HTTP", "Record deleted: " + id);
    }
    else
    {
        res.set_content("{\"error\": \"record not found\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::NOT_FOUND);
        printResponse(res.status, "{\"error\": \"record not found\"}");
        LOG_WARN("HTTP", "Record not found for delete: " + id);
    }
}
