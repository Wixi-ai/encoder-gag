#pragma once

#include <string>
#include <vector>
#include <so_5/all.hpp>

struct msg_create_record {
    std::string id;
    std::string file_path;
    std::string request_body;
};

struct msg_get_records {
    int request_id;
    so_5::mbox_t reply_to;
};

struct msg_get_records_response {
    int request_id;
    std::vector<std::pair<std::string, std::string>> records;
};
