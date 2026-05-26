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

struct msg_get_record_by_id {
    std::string id;
    int request_id;
    so_5::mbox_t reply_to;
};

struct msg_get_record_by_id_response {
    int request_id;
    bool found;
    std::string id;
    std::string file_path;
    std::string created_at;
};

struct msg_delete_record_by_id {
    std::string id;
    int request_id;
    so_5::mbox_t reply_to;
};

struct msg_delete_record_by_id_response {
    int request_id;
    bool success;
    std::string error_message;
};
