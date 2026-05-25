#pragma once

#include <string>
#include <vector>
#include <so_5/all.hpp>

struct msg_create_record {
    std::string id;
    std::string file_path;
    std::string request_body;
    so_5::mbox_t reply_to;
};

struct msg_create_response {
    std::string id;
    bool success;
    std::string error_message;
};

struct msg_get_records {
    int request_id;
    so_5::mbox_t reply_to;
};

struct msg_get_records_response {
    int request_id;
    std::vector<std::pair<std::string, std::string>> records;
};
