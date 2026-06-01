#include "../../include/agents/http_agent.hpp"
#include "../../include/colors.hpp"
#include "../../include/utils.hpp"
#include "../../include/logger.hpp"
#include "../../include/constants.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
#include <tuple>

using json = nlohmann::json;

extern std::chrono::steady_clock::time_point start_time;

static bool validatePostRequest(const json &j, httplib::Response &res, RecordCreateRequest &request) {
    if (!j.contains("id") || !j["id"].is_string()) {
        res.set_content("{\"error\": \"missing or invalid 'id' field\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }
    request.id = j["id"].get<std::string>();
    if (!is_valid_uuid(request.id)) {
        res.set_content("{\"error\": \"invalid uuid format\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }
    
    if (!j.contains("block_size") || !j["block_size"].is_number_integer()) {
        res.set_content("{\"error\": \"missing or invalid 'block_size' field\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }
    request.block_size = j["block_size"].get<int>();
    if (request.block_size <= 0) {
        res.set_content("{\"error\": \"block_size must be positive integer\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }
    
    if (!j.contains("fblock") || !j["fblock"].is_number_integer()) {
        res.set_content("{\"error\": \"missing or invalid 'fblock' field\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }
    request.fblock = j["fblock"].get<int>();
    
    if (!j.contains("streams") || !j["streams"].is_array()) {
        res.set_content("{\"error\": \"missing or invalid 'streams' field (must be array)\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        return false;
    }
    
    for (const auto& stream_json : j["streams"]) {
        Stream stream;
        
        if (!stream_json.contains("type") || !stream_json["type"].is_string()) {
            res.set_content("{\"error\": \"each stream must have string 'type' field (video/audio)\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
            return false;
        }
        stream.type = stream_json["type"].get<std::string>();
        if (stream.type != "video" && stream.type != "audio") {
            res.set_content("{\"error\": \"stream type must be 'video' or 'audio'\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
            return false;
        }
        
        if (!stream_json.contains("id") || !stream_json["id"].is_number_integer()) {
            res.set_content("{\"error\": \"each stream must have integer 'id' field\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
            return false;
        }
        stream.id = stream_json["id"].get<int>();
        
        if (!stream_json.contains("files") || !stream_json["files"].is_array()) {
            res.set_content("{\"error\": \"each stream must have 'files' array\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
            return false;
        }
        
        for (const auto& file_json : stream_json["files"]) {
            FileInfo file;
            
            if (!file_json.contains("begin") || !file_json["begin"].is_string()) {
                res.set_content("{\"error\": \"each file must have 'begin' timestamp\"}", "application/json");
                res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
                return false;
            }
            file.begin = file_json["begin"].get<std::string>();
            
            if (!file_json.contains("path") || !file_json["path"].is_string()) {
                res.set_content("{\"error\": \"each file must have 'path' string\"}", "application/json");
                res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
                return false;
            }
            file.path = file_json["path"].get<std::string>();
            
            stream.files.push_back(file);
        }
        
        request.streams.push_back(stream);
    }
    
    return true;
}

void http_agent_t::handlePost(const httplib::Request &req, httplib::Response &res)
{
    if (req.body.empty()) {
        res.set_content("{\"error\": \"empty request body\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        printResponse(res.status, "{\"error\": \"empty request body\"}");
        LOG_WARN("HTTP", "Empty request body");
        return;
    }

    try {
        json j = json::parse(req.body);
        RecordCreateRequest request;
        
        if (!validatePostRequest(j, res, request)) {
            printResponse(res.status, res.body);
            return;
        }

        m_request_counter++;
        printRequest("POST", "/api/v1/records", m_request_counter, request.id.substr(0, 8), req.body);
        LOG_INFO("HTTP", "POST #" + std::to_string(m_request_counter) + " id=" + request.id);

        m_records_cache.invalidateAll();

        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();
        {
            std::lock_guard<std::mutex> lock(m_creates_mutex);
            m_pending_creates[request.id] = promise;
        }

        msg_create_record msg;
        msg.id = request.id;
        msg.streams = request.streams;
        msg.block_size = request.block_size;
        msg.fblock = request.fblock;
        msg.reply_to = so_direct_mbox();
        so_5::send<msg_create_record>(m_db_mbox, msg);

        std::cout << COLOR_HTTP << "  -> DB Agent | sent, waiting for response" << COLOR_RESET << std::endl;

        if (future.wait_for(std::chrono::seconds(config::TIMEOUT_SECONDS)) != std::future_status::ready) {
            std::cout << COLOR_RED << "  [ERROR] Timeout waiting for DB response" << COLOR_RESET << std::endl;
            res.set_content("{\"error\": \"database timeout\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::GATEWAY_TIMEOUT);
            printResponse(res.status, "{\"error\": \"database timeout\"}");
            LOG_ERROR("HTTP", "Timeout for id=" + request.id);
            return;
        }

        if (future.get()) {
            std::string response = "{\"status\": \"created\", \"id\": \"" + request.id + "\"}";
            res.set_content(response, "application/json");
            res.status = static_cast<int>(HttpStatus::CREATED);
            printResponse(res.status, response);
            LOG_INFO("HTTP", "Record created: " + request.id);
        } else {
            res.set_content("{\"error\": \"record already exists\"}", "application/json");
            res.status = static_cast<int>(HttpStatus::CONFLICT);
            printResponse(res.status, "{\"error\": \"record already exists\"}");
            LOG_WARN("HTTP", "Duplicate record: " + request.id);
        }
    } catch (const json::parse_error& e) {
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
    
    std::string codec = "";
    std::string from_date = "";
    std::string to_date = "";
    std::string file_path = "";

    if (req.has_param("limit")) {
        limit = std::stoi(req.get_param_value("limit"));
        if (limit <= 0) limit = config::DEFAULT_LIMIT;
        if (limit > config::MAX_LIMIT) limit = config::MAX_LIMIT;
    }
    if (req.has_param("offset")) {
        offset = std::stoi(req.get_param_value("offset"));
        if (offset < 0) offset = 0;
    }
    if (req.has_param("sort_by")) {
        sort_by = req.get_param_value("sort_by");
        if (sort_by != "id" && sort_by != "file_path" && sort_by != "created_at")
            sort_by = "created_at";
    }
    if (req.has_param("sort_order")) {
        sort_order = req.get_param_value("sort_order");
        if (sort_order != "asc" && sort_order != "desc")
            sort_order = "asc";
    }
    
    if (req.has_param("codec")) codec = req.get_param_value("codec");
    if (req.has_param("from_date")) from_date = req.get_param_value("from_date");
    if (req.has_param("to_date")) to_date = req.get_param_value("to_date");
    if (req.has_param("file_path")) file_path = req.get_param_value("file_path");

    int req_id = ++m_request_id_counter;
    
    std::string cache_key = "limit=" + std::to_string(limit) + "&offset=" + std::to_string(offset) 
                          + "&sort_by=" + sort_by + "&sort_order=" + sort_order
                          + "&codec=" + codec + "&from_date=" + from_date 
                          + "&to_date=" + to_date + "&file_path=" + file_path;
    
    std::string cached_response;
    if (m_records_cache.get(cache_key, cached_response)) {
        std::cout << COLOR_HTTP << "  [CACHE HIT] Returning cached response" << COLOR_RESET << std::endl;
        res.set_content(cached_response, "application/json");
        res.status = static_cast<int>(HttpStatus::OK);
        printResponse(res.status, cached_response.substr(0, 60) + (cached_response.size() > 60 ? "..." : ""));
        return;
    }
    
    std::cout << COLOR_HTTP << "  [CACHE MISS] Fetching from DB" << COLOR_RESET << std::endl;
    
    printRequest("GET", "/api/v1/records?" + cache_key, req_id, "", "");
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

    if (future.wait_for(std::chrono::seconds(config::DB_TIMEOUT_SECONDS)) != std::future_status::ready) {
        res.set_content("{\"error\": \"timeout\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::GATEWAY_TIMEOUT);
        printResponse(res.status, "{\"error\": \"timeout\"}");
        LOG_ERROR("HTTP", "Timeout getting all records");
        return;
    }

    auto response = future.get();
    json j = json::array();
    for (const auto& rec : response.records) {
        j.push_back({{"id", std::get<0>(rec)}, {"start", std::get<1>(rec)}, {"finish", std::get<2>(rec)}});
    }
    
    json result = {
        {"total", response.total},
        {"limit", response.limit},
        {"offset", response.offset},
        {"sort_by", sort_by},
        {"sort_order", sort_order},
        {"records", j}
    };
    
    if (!codec.empty()) result["codec"] = codec;
    if (!from_date.empty()) result["from_date"] = from_date;
    if (!to_date.empty()) result["to_date"] = to_date;
    if (!file_path.empty()) result["file_path"] = file_path;
    
    std::string response_str = result.dump();
    m_records_cache.set(cache_key, response_str);
    
    res.set_content(response_str, "application/json");
    res.status = static_cast<int>(HttpStatus::OK);
    printResponse(res.status, response_str.substr(0, 60) + (response_str.size() > 60 ? "..." : ""));
    LOG_INFO("HTTP", "Returned " + std::to_string(response.records.size()) + " records (total: " + std::to_string(response.total) + ")");
}

void http_agent_t::handleGetById(const std::string &id, httplib::Response &res)
{
    if (!is_valid_uuid(id)) {
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

    if (future.wait_for(std::chrono::seconds(config::DB_TIMEOUT_SECONDS)) != std::future_status::ready) {
        res.set_content("{\"error\": \"timeout\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::GATEWAY_TIMEOUT);
        printResponse(res.status, "{\"error\": \"timeout\"}");
        LOG_ERROR("HTTP", "Timeout getting record: " + id);
        return;
    }

    auto response = future.get();
    if (response.found) {
        json j = {{"id", response.id}, {"file_path", response.file_path}, {"created_at", response.created_at}};
        res.set_content(j.dump(), "application/json");
        res.status = static_cast<int>(HttpStatus::OK);
        printResponse(res.status, j.dump());
        LOG_INFO("HTTP", "Record found: " + id);
    } else {
        res.set_content("{\"error\": \"record not found\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::NOT_FOUND);
        printResponse(res.status, "{\"error\": \"record not found\"}");
        LOG_WARN("HTTP", "Record not found: " + id);
    }
}

void http_agent_t::handleDelete(const std::string &id, httplib::Response &res)
{
    if (!is_valid_uuid(id)) {
        res.set_content("{\"error\": \"invalid uuid format\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::BAD_REQUEST);
        printResponse(res.status, "{\"error\": \"invalid uuid format\"}");
        LOG_WARN("HTTP", "Invalid UUID for delete: " + id);
        return;
    }

    int req_id = ++m_request_id_counter;
    printRequest("DELETE", "/api/v1/records/" + id, req_id, id.substr(0, 8), "");
    LOG_DEBUG("HTTP", "DELETE record: " + id);

    m_records_cache.invalidateAll();

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

    if (future.wait_for(std::chrono::seconds(config::DB_TIMEOUT_SECONDS)) != std::future_status::ready) {
        res.set_content("{\"error\": \"timeout\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::GATEWAY_TIMEOUT);
        printResponse(res.status, "{\"error\": \"timeout\"}");
        LOG_ERROR("HTTP", "Timeout deleting record: " + id);
        return;
    }

    auto response = future.get();
    if (response.success) {
        res.set_content("", "application/json");
        res.status = static_cast<int>(HttpStatus::NO_CONTENT);
        printResponse(res.status, "");
        LOG_INFO("HTTP", "Record deleted: " + id);
    } else {
        res.set_content("{\"error\": \"record not found\"}", "application/json");
        res.status = static_cast<int>(HttpStatus::NOT_FOUND);
        printResponse(res.status, "{\"error\": \"record not found\"}");
        LOG_WARN("HTTP", "Record not found for delete: " + id);
    }
}
