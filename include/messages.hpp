#pragma once

#include <string>
#include <vector>
#include <so_5/all.hpp>

struct TimeBase {
    int num;
    int den;
};

struct FileInfo {
    std::string begin;   // ISO 8601 timestamp
    std::string path;    // путь к файлу
};

struct Stream {
    std::string type;           // "video" или "audio"
    int id;
    std::vector<FileInfo> files;
};

struct RecordCreateRequest {
    std::string id;              // UUID
    int block_size;
    int fblock;                  // integer по ТЗ
    std::vector<Stream> streams;
};

// Сообщения для акторов
struct msg_create_record {
    std::string id;
    std::vector<Stream> streams;
    int block_size;
    int fblock;
    so_5::mbox_t reply_to;
};

struct msg_create_response {
    std::string id;
    bool success;
    std::string error_message;
};

struct msg_get_records {
    int request_id;
    int limit;
    int offset;
    std::string sort_by;
    std::string sort_order;
    std::string codec;
    std::string from_date;
    std::string to_date;
    std::string file_path;
    so_5::mbox_t reply_to;
};

struct msg_get_records_response {
    int request_id;
    int total;
    int limit;
    int offset;
    std::vector<std::tuple<std::string, std::string, std::string>> records;
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

// Временные сообщения для ffmpeg (оставляем как есть)
struct msg_process_video {
    std::string record_id;
    std::string file_path;
    int request_id;
    so_5::mbox_t reply_to;
};

struct msg_video_params {
    int request_id;
    bool success;
    std::string record_id;
    std::string codec;
    int width;
    int height;
    double duration;
    std::string error_message;
};

struct VaaBlock {
    int index;
    std::string type;
    int64_t pts;
    int64_t duration;
    std::string data;
};

struct msg_create_vaa_blocks {
    std::string record_id;
    std::vector<VaaBlock> blocks;
    int request_id;
    so_5::mbox_t reply_to;
};

struct msg_vaa_blocks_response {
    int request_id;
    bool success;
    std::string error_message;
};
