#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <so_5/all.hpp>

// ========== Структуры для файлов и потоков ==========

struct FileInfo {
    std::string begin;
    std::string path;
};

struct Stream {
    std::string type;      // "video" или "audio"
    int id;
    std::vector<FileInfo> files;
};

struct RecordCreateRequest {
    std::string id;
    int block_size;
    int fblock;
    std::vector<Stream> streams;
};

// ========== Структуры для VAA блоков ==========

struct VaaBlock {
    int index;
    std::string type;      // "video" или "audio"
    int64_t pts;           // Presentation timestamp
    int64_t duration;      // Длительность в миллисекундах
    std::string data;      // Base64-encoded данные
};

// ========== Структуры для видео/аудио параметров ==========

struct TimeBase {
    int num;
    int den;
};

struct VideoStream {
    int id;
    std::string codec;
    int width;
    int height;
    TimeBase time_base;
    std::string extra;     // Base64-encoded extra data
};

struct AudioStream {
    int id;
    std::string codec;
    int sample_rate;
    int channels;
    std::string channel_layout;
    TimeBase time_base;
    std::string extra;     // Base64-encoded extra data
};

// ========== Сообщения между агентами ==========

// HTTP -> DB: создание записи
struct msg_create_record {
    std::string id;
    std::vector<Stream> streams;
    int block_size;
    int fblock;
    so_5::mbox_t reply_to;
};

// DB -> HTTP: ответ на создание
struct msg_create_response {
    std::string id;
    bool success;
    std::string error_message;
};

// HTTP -> DB: получить список записей
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

// DB -> HTTP: ответ со списком записей
struct msg_get_records_response {
    int request_id;
    int total;
    int limit;
    int offset;
    std::vector<std::tuple<std::string, std::string, std::string>> records; // id, start, finish
};

// HTTP -> DB: получить запись по ID
struct msg_get_record_by_id {
    std::string id;
    int request_id;
    so_5::mbox_t reply_to;
};

// DB -> HTTP: ответ с записью
struct msg_get_record_by_id_response {
    int request_id;
    bool found;
    std::string id;
    std::string created_at;
    std::vector<VideoStream> video;
    std::vector<AudioStream> audio;
};

// HTTP -> DB: удалить запись
struct msg_delete_record_by_id {
    std::string id;
    int request_id;
    so_5::mbox_t reply_to;
};

// DB -> HTTP: ответ на удаление
struct msg_delete_record_by_id_response {
    int request_id;
    bool success;
    std::string error_message;
};

// HTTP -> FFmpeg: обработать видео
struct msg_process_video {
    std::string record_id;
    std::string file_path;
    int request_id;
    so_5::mbox_t reply_to;
};

// FFmpeg -> HTTP: параметры видео
struct msg_video_params {
    int request_id;
    std::string record_id;
    bool success;
    std::string codec;
    int width;
    int height;
    double duration;
    std::string error_message;
};

// FFmpeg -> DB: сохранить VAA блоки
struct msg_save_vaa_blocks {
    std::string record_id;
    std::vector<VaaBlock> blocks;
    int request_id;
    so_5::mbox_t reply_to;
};

// DB -> FFmpeg: ответ на сохранение VAA блоков
struct msg_save_vaa_blocks_response {
    int request_id;
    bool success;
    std::string error_message;
};

// FFmpeg -> HTTP: ответ о создании VAA блоков
struct msg_vaa_blocks_response {
    int request_id;
    bool success;
    std::string error_message;
};

// HTTP -> FFmpeg: создать VAA блоки
struct msg_create_vaa_blocks {
    std::string record_id;
    std::vector<VaaBlock> blocks;
    int request_id;
    so_5::mbox_t reply_to;
};

// HTTP -> DB: получить VAA блоки
struct msg_get_vaa_blocks {
    std::string record_id;
    int request_id;
    int limit;
    int offset;
    so_5::mbox_t reply_to;
};

// DB -> HTTP: ответ с VAA блоками
struct msg_get_vaa_blocks_response {
    int request_id;
    bool found;
    std::string record_id;
    int total;
    int limit;
    int offset;
    std::vector<VaaBlock> blocks;
};
